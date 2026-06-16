# Makefile 自動標頭依賴(.d 檔)筆記

## 問題:make 看不見 `#include`

make 只認 Makefile 裡聲明的依賴,不會解析 C 原始碼。如果 `.c` 檔 include 了
`math.h`,但規則裡沒把 `math.h` 列為前置依賴,那麼修改標頭後 make 比對時間戳
只看 `.c`,會誤判「已是最新」而跳過重編——舊的 `.o` 被鏈接到新的介面上,
產生悄無聲息的不匹配 bug。

手寫依賴可以解決:

```makefile
function_pointer.o: function_pointer.c math.h
	$(CC) $(CFLAGS) -c $<
```

但手寫在大項目裡不可靠:

- **依賴是傳遞的**:標頭又 include 標頭,真實依賴鏈要人工追到底;
- **失效無聲**:加了 `#include` 忘了改 Makefile,當下編譯照樣通過,
  幾週後才以詭異方式爆發(「make clean 之後就好了」);
- **規模成本**:幾百條規則無人校驗,每次重構都要同步維護。

解法:讓編譯器(本來就要追蹤 include 鏈)順手把依賴吐出來。

## 自動依賴的完整 Makefile

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -g -MMD -MP

TARGET = app
OBJS = function_pointer.o math.o
DEPS = $(OBJS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

-include $(DEPS)

clean:
	rm -f $(TARGET) $(OBJS) $(DEPS)

.PHONY: all clean
```

要點:模式規則 `%.o: %.c` **不寫任何標頭依賴**,標頭依賴全部由 `.d` 檔提供。

## -M 選項家族

| 選項   | 作用                                        |
|--------|---------------------------------------------|
| `-M`   | 只輸出依賴規則到 stdout,不編譯,含系統標頭   |
| `-MM`  | 同上,但排除系統標頭                         |
| `-MD`  | 正常編譯,同時把規則寫進 `.d` 檔(含系統標頭) |
| `-MMD` | 正常編譯,同時寫 `.d` 檔,排除系統標頭        |
| `-MP`  | 額外為每個標頭生成一條空規則(見下)          |

實務用 `-MMD`:系統標頭幾乎不變,沒必要追蹤。關鍵優勢是 `.d` 是**編譯的
副產品**——一條 `gcc -MMD -c foo.c` 同時產出 `foo.o` 和 `foo.d`,零額外開銷。

## .d 檔的內容

`gcc -MMD -MP -c function_pointer.c` 生成的 `function_pointer.d`:

```makefile
function_pointer.o: function_pointer.c math.h
math.h:
```

- 第一行列出這次編譯**實際讀取**的每個檔案,傳遞依賴自動展開;
- 規則只有依賴、沒有配方(recipe)——合法,意思是「給該目標追加依賴」;
- `math.h:` 那行來自 `-MP`(見後文)。

## .d 怎麼和編譯指令接起來

make 允許**同一目標出現在多條規則**:前置依賴取聯集累加,但配方只能有一條。
讀完 Makefile(含 `-include` 進來的 `.d`)後,make 眼中的規則是合體:

```makefile
%.o: %.c                                  # Makefile:提供配方
	$(CC) $(CFLAGS) -c $<
function_pointer.o: function_pointer.c math.h   # .d 檔:只提供依賴
```

職責分離:**`.d` 回答「什麼時候該重建」,模式規則回答「怎麼重建」。**

一次 build 的流程(假設改了 `math.h`):

1. `.d` 告訴 make:`function_pointer.o` 依賴 `math.h`;
2. `math.h` 較新 → 目標過期;
3. 明確規則無配方 → make 搜模式規則,`%.o: %.c` 匹配,執行其配方;
4. 這次編譯寫出新 `.d`,供下一次 make 使用。

細節:`$<` 展開為第一個前置依賴。模式規則中 make 保證**模式匹配出的那個
依賴**(`.c` 檔)排第一,其他規則補進的依賴排後面,所以 `$<` 永遠是 `.c`。

時序:`-include` 讀到的是**上一次編譯**生成的 `.d`,最多「晚一輪」但不會
漏編——新增 include 必然伴隨某個檔案被修改,該修改本身就觸發重編並刷新 `.d`。

## `-include $(OBJS:.o=.d)` 逐層拆解

**`$(OBJS:.o=.d)`** — 替換引用(substitution reference),把每個字的 `.o`
換成 `.d`,等價於 `$(patsubst %.o,%.d,$(OBJS))`。`.d` 清單永遠跟著 `OBJS`
走,不用維護第二份清單。

**`include`** — 把檔案內容原地貼進 Makefile。發生在 make 的**讀取階段**,
依賴圖在任何目標建置之前就已完整。

**前綴 `-`** — 找不到檔案時靜默跳過。沒有它,第一次編譯(或 clean 後)
`.d` 不存在,普通 `include` 會報錯停止。安全性在於:`.d` 缺失的唯一情形是
對應的 `.o` 也不存在,而此時本來就必須全量編譯,不需要依賴資訊。
(同義詞 `sinclude`;老式做法是專門寫 `%.d: %.c` 規則配普通 `include`,
讓 make 重建被包含檔案後重啟自己,複雜且慢,已被現代做法取代。)

## -MP:防「標頭被刪除」死鎖

### 沒有 -MP 的事故鏈

1. 編譯過一次,`.d` 記下 `foo.o` 依賴 `bar.h`;
2. 重構:`foo.c` 不再 include `bar.h`,並刪掉 `bar.h`(源碼層面完全正確);
3. 再跑 make:讀入舊 `.d`,要找 `bar.h` —— 檔案不存在又無規則可生成:

   ```
   make: *** No rule to make target 'bar.h', needed by 'foo.o'.  Stop.
   ```

死鎖:消除過時記錄需要重編刷新 `.d`,但 make 連編譯都不肯開始,
只能手動刪 `.d` 或 `make clean`。

### 空規則為何能解鎖

`-MP` 為每個標頭生成 `bar.h:` 這樣的空規則。GNU make 的語義:

> 若規則沒有依賴也沒有配方,且目標檔案不存在,make 認定每次處理該規則時
> 目標都被更新了;所有依賴它的目標必然重新執行配方。

make 更新目標的演算法走一遍:

1. 找到規則 `bar.h:`;
2. 無依賴,無事可做;
3. 判斷是否重建:**目標檔案不存在 → 需要重建**;
4. 執行(空)配方,成功。關鍵:**make 不會回頭檢查檔案是否真的被造出來**,
   信任模型是「規則跑成功 = 目標已更新」→ `bar.h` 記為「本輪剛重建」;
5. 回到 `foo.o`:**有依賴在本輪被重建 → 目標必須重建** → 觸發重編;
6. 重編寫出新 `.d`,過時記錄被沖掉,問題自我修復。

而標頭**存在**時,第 3 步判據不成立,空規則完全隱形。所以它是個精確開關:
標頭在 → 無作用;標頭消失 → 不報錯且強制重編引用過它的 `.o`。

若標頭是被誤刪的(`.c` 還在 include),重編會得到編譯器的
`fatal error: bar.h: No such file or directory`——比 make 那句
「no rule to make target」清晰得多,錯誤被下沉到診斷更準的層級。

### 兩個細節

- 名字裡的 "phony" 是 GCC 文件的叫法,機制上和 `.PHONY` 無關,
  生成的就只是普通空規則;
- `-MP` 必須搭配 `-MD`/`-MMD` 使用,慣用組合永遠成對:
  **`-MMD` 解決「標頭改了要重編」,`-MP` 解決「標頭沒了別卡死」。**

### 同一機制的另一用法:FORCE 慣用法

「不存在的空規則目標 = 永遠視為剛更新」是 make 的通用機制:

```makefile
version.o: FORCE        # FORCE 永遠「剛被更新」→ version.o 每次必重編
	cc -DBUILD_TIME=$(shell date +%s) -c version.c
FORCE:
```

`-MP` 生成的空規則本質是「條件版 FORCE」——平時被真實檔案壓住不生效,
檔案一消失就變成強制重建的扳機。

## 收尾

- `.d` 和 `.o`、執行檔一樣是建置產物:`clean` 要刪,`.gitignore` 要加;
- 整個閉環:每次編譯刷新 `.d` → 依賴清單永遠由程式碼生成、與程式碼同步,
  消除「人記得更新 Makefile」這個不可靠環節。
