# My Html Library (myhtml2)

## 情報

| Item | Value |
|:---|:---|
| 作成日 | |
| バージョン | 2.0.0 (Updating) |
| プログラミング言語 | C |


---

## 使い方

1. `HtmlReadObjectFrom<source>()` でHTMLデータを読み込みます
2. `HtmlFindObject()` や `HtmlSelectObject()` でターゲットとなるオブジェクトを取得します
3. `HtmlGetObject<data>()` でオブジェクトデータを取得します
4. 最後に `HtmlDestroyObject()` でリソースを解放します

### サンプルコード

```c
#include "myhtml2/myhtml.h"

int main(int argc, char** argv) {
  // 1. HTML読み込み
  HtmlObject* doc = HtmlReadObjectFromFile("sample.html");

  // 2. ターゲット要素の取得 タグ名div idがmain のタグ
  HtmlObject* target = HtmlFindObject(doc, "div#main");

  // 3. テキスト取得
  HtmlStream stream = HtmlCreateStreamBuffer(64);
  HtmlGetObjectText(target, &stream);

  printf("Text: %s\n", HtmlGetStreamString(&stream));

  // 4. オブジェクト削除
  HtmlDestroyObject(doc);
}
```

---

この流れをREADMEの「使用方法」や「クイックスタート」セクションに掲載すると、ユーザーが理解しやすくなります。  
他にもAPIリストやサンプルプロジェクトがあれば、合わせて紹介すると親切です。

他に記載したい内容や、加えたい説明があれば教えてください！