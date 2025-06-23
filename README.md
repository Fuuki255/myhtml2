# myhtml2

C言語で書かれた軽量・高速なHTMLパーサ＆セレクタライブラリ

| バージョン    | 2.0.0         |
|:-------------|:--------------|
| プログラマー  | ふうき255      |
| 完成日時      | 24 Jun 2025   |

## 特徴

- 文字列、ファイル、ストリーム、libcurl など多様な入力ソースからHTMLをパース
- タグ名、クラスや ID による要素検索
- 属性値やテキストの簡単取得
- HTMLの再出力やファイル書き込みも可能
- 明確なメモリ管理
- 詳しいデバックメッセージが表示され、NULL処理が忘れても平気い、メッセージのオフも可能

---

## サンプル

```c
#include <curl/curl.h>        // libcurl の拡張機能を使用するには myhtml.h 前に定義する必要がある
#include "myhtml2/myhtml.h"

int main(int argc, char** argv) {
  // libcurl 初期化
  CURL* curl = curl_easy_init();

  // HTML 取得
  HtmlObject* doc = HtmlReadObjectFromCURL(curl, "https://example.com");

  // 検索1 : title
  HtmlObject* tagTitle = HtmlFindObject(doc, "title");
  printf("Title %s\n", HtmlGetObjectInnerText(tagTitle));

  // 検索2 : <meta charset>
  HtmlObject* tagMeta = HtmlFindObject(doc, "meta");
  printf("charset: %s\n", HtmlGetObjectAttributeValue(tagMeta, "charset"));

  // 検索3 : テキスト表示
  HtmlObject* tagBody = HtmlFindObject(doc, "body[-1]");    // -1 は逆方向から探すため、後ろにいる body を効率的に探せる（ここに大した違いがないけど）

  HtmlStream stream = HtmlCreateStreamBuffer(1024);     // HtmlGetObjectText の結果は新しい文字列に保存するため、ここにバッファーを作る
  HtmlGetObjectText(tagBody, &stream);                  // HtmlGetObjectText で body のすべてのテキストを stream に書き込む
  printf("Text:\n%s\n\n", HtmlGetStreamString(&stream));  // stream から書き込んだデータをプリント

  HtmlDestroyStream(&stream);     // stream 削除

  // 検索4 : マルチ検索1
  HtmlSelect select = HtmlCreateSelect(doc, "meta");    // myhtml2 の検索コアとなった HtmlSelect、イテレータ構造で必要な分だけ検索することができます
  HtmlObject* object;

  printf("All <meta> in html:\n");
  while ((object = HtmlNextSelect(&select))) {
    printf("  %s\n", HtmlWriteObjectToString(object));  // HtmlObject を HTML 文字列に変換する。戻り値は自動的に解放されるのでfree不要
  }
  
  HtmlDestroySelect(&select);   // HtmlSelect の使用後は削除する必要があります

  // 検索5 : マルチ検索2
  HtmlArray array = HtmlFindAllObjects(doc, "p");     // HtmlSelect の結果を配列にまとめるメソッド、 出力の HtmlArray は同様に削除する必要がある
  
  printf("All <p> in html:\n");
  for (int i = 0; i < array.length; i++) {
    printf("  %s\n", HtmlWriteObjectToString(array.values[i]));
  }

  HtmlDestroyArray(&array);

  // (オプション) HTML 出力
  HtmlWriteObjectToFile(doc, "output.html");

  // クリーンアップ
  HtmlDestroyObject(doc);
  // Get, Find, Next などで得た HtmlObject は削除する必要がない

  curl_easy_cleanup(curl);
  return 0;
}
```

---

## 使い方

### 1. HTMLの読み込み

```c
HtmlObject* obj;

// 文字列から
obj = HtmlReadObjectFromString("<html>...</html>");

// ファイルから
obj = HtmlReadObjectFromFile("example.html");

// FILE* から
obj = HtmlReadObjectFromFileObject(fp);

// ストリームから（カスタム実装）
obj = HtmlReadObjectFromStream(htmlStream);

// libcurl で取得（拡張）
obj = HtmlReadObjectFromCURL(curl, "https://example.com");
```

---

### 2. オブジェクト検索

```c
// イテレータ検索
HtmlSearch* search = HtmlCreateSearch(obj, "div.classname");

// 最初を検索 (HtmlSearch ベース)
HtmlObject* found = HtmlFindObject(search, "span#id");

// すべてを検索し、配列に保存する (HtmlSearch ベース)
HtmlArray* select = HtmlFindAllObject(search, ".classname");
```

---

### 3. データ取得

```c
// 属性データを取得
const char* value = HtmlGetObjectAttributeValue(found, "href");

// 内部テキストを取得
const char* innerText = HtmlGetObjectInnerText(found);

// 内部すべてのタグのテキストを取得
HtmlGetObjectText(found, &htmlStream);
```

---

### 4. (オプション) HTMLの出力

```c
// ファイルに書き込む
HtmlWriteObjectToFile(doc, "output.html");

// 既存の FILE に書き込む
HtmlWriteObjectToFileObject(doc, file);

// 文字列に変換、戻り値は自動的に解放されるのでfree不要
const char* str = HtmlWriteObjectToString(doc);

// カスタマイズオブジェクトに書き込む
HtmlWriteObjectToStream(doc, &htmlStream);
```

---

### 5. メモリ解放

```c
// HtmlObject 削除
HtmlDestroyObject(obj);

// HtmlArray 削除
HtmlDestroyArray(array);

// HtmlSelect 削除
HtmlDestroySelect(select);
```

---

## ライブラリの歴史

- **myhtml（beta1.0.0, 2023年）**
  - フォルダサイズ: 63.5KB
  - 最初のバージョンは myhtml（すべて小文字）として開発され、HTMLパースや検索など完全な機能を持っていた。
  - ただし、ファイルが多数に分割されており、利用時の簡潔さに欠けていた。

- **MyHtml (1.0.0, 2024年)**
  - フォルダサイズ: 29.1KB
  - コードを大幅に簡潔化し、ファイル数も減らして再設計。
  - しかし、機能面では `beta1.0.0` の myhtml よりも検索機能が限定的で、中途半端な部分もあった。

- **myhtml2（2.0.0, 2025年）**
  - フォルダサイズ: 58.3KB
  - より進化した設計で、ファイル分割を最小限に抑えつつ、安定性・パフォーマンス・機能性のすべてが過去最高レベルに。
  - 検索やストリーム、libcurl対応など多くの新機能を搭載。

---

## ライセンス

MIT License
