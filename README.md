# myhtml2 - 高速なHTMLパーサー

C言語で書かれた軽量・高速なHTMLライブラリ、以前のバージョン (beta1.0 から 1.0) と比べてより高速、高機能、高安定性、読みやすいなどで全方位的にアップグレードされています

| バージョン    | 2.1.0         |
|:-------------|:--------------|
| プログラマー  | ふうき255      |
| 完成日時      | 24 Jun 2025   |

## 特徴

- 文字列、ファイル、カスタマイズリーダー (HtmlStream) や libcurl などからHTMLを解析できる
- 各HTMLバージョンの対応性、BeautifulSoup4 を参考したのエラードキュメント処理
- タグ名、クラス、ID や検索番目 [index] による要素検索
- 外部参考いらない、唯一参考 libcurl が導入していないと対応のメソッドを作成しない
- HtmlObject 書き込み可能
- 簡潔なコードができる
- デバックの表示、全メソッドに NULL 入力が処理が対処され、デバックメッセージをオフも可能

---

## インストール

1. `git clone https://www.github.com/Fuuki255/myhtml2` で myhtml2 をダウンロード
2. Cプログラムに `#include "myhtml2/myhtml.h"` ヘッダーでライブラリを導入すればいい

---

## サンプル

```c
#include <curl/curl.h>        // libcurl の拡張機能を使用するには myhtml.h 前に定義する必要がある
#include "myhtml.h"

int main(int argc, char** argv) {
  /* libcurl 初期化 */
  CURL* curl = curl_easy_init();

  /* https://example.com からHTML を取得する */
  HtmlObject* doc = HtmlReadObjectFromCURL(curl, "https://example.com");

  /* 検索実例1: <title></title> */
  HtmlObject* tagTitle = HtmlFindObject(doc, "title");
  printf("Title %s\n", HtmlGetObjectInnerText(tagTitle));

  /* 検索実例2: body プリント */
  HtmlObject* tagBody = HtmlFindObject(doc, "body[-1] /* インデックス -1 に設定することで逆方向の検索ができます */);

  // GetObjectInnerText() と違って、GetObjectText() は子オブジェクトのテキストも含めたゲットなので新しいバッファが必要
  // GetStreamString() は文字列系ストリームをC文字列に変更する
  HtmlStream streamText = HtmlCreateStreamBuffer(1024);
  printf("Text:\n%s\n", HtmlGetStreamString(&streamText));

  // HtmlStream の使用後は削除が必要
  HtmlDestroyStream(&streamText);

  /* (オプション) HTML 出力 */
  HtmlWriteObjectToFile(doc, "output.html");

  /* クリーンアップ */
  HtmlDestroyObject(doc);

  curl_easy_cleanup(curl);
  return 0;
}
```

---

## 使い方

### 1. HTMLの読み込み

```c
HtmlObject* obj;
FILE* fp;
HtmlStream htmlStream;

// 文字列から
obj = HtmlReadObjectFromString("<html>...</html>");

// ファイルから
obj = HtmlReadObjectFromFile("example.html");

// FILE* から
obj = HtmlReadObjectFromFileObject(fp);

// ストリームから（カスタム実装）
obj = HtmlReadObjectFromStream(&htmlStream);

// libcurl で取得（拡張）
obj = HtmlReadObjectFromCURL(curl, "https://example.com");
```

---

### 2. オブジェクト検索

"tagName.className#tagId[index] (nextPattern ...)" というバタンを使用した検索

- tagName はタグ名で検索 （大小文字気にせず）
- className はクラス名で検索（大小文字区別あり）
- tagId は ID で検索（大小文字区別あり）
- [index] はその検索結果から index 番目の結果を戻す (、整数は正順から、-1 は逆番から

* tagName, className や tagId 任意の一つが必要
* [index] はオプション
* CreateSearch(), FindObject(), FindAllObjects() の検索結果は同じで結果方式が違う


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
HtmlStream* stream = HtmlCreateStream(&stream);
HtmlGetObjectText(found, &stream);

HtmlDestroyStream(&stream);
```

---

### 4. (オプション) オブジェクト変更

```c
// テキスト設定
HtmlObjectSetInnerText(tagH1, "Hello, World");

// 属性設定
HtmlSetObjectAtterValue(tagMeta, "charset", "utf-8");

// オブジェクト追加
HtmlAddObjectChild(object, child);
```

---

### 5. (オプション) HTMLの出力

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

### 6. メモリ解放

```c
// HtmlObject 削除
HtmlDestroyObject(obj);

// HtmlStream 削除
HtmlDestroyStream(&stream);

// HtmlArray 削除
HtmlDestroyArray(array);

// HtmlSelect 削除
HtmlDestroySelect(select);
```

---

## アップデート予定

- マルチオブジェクト検索 ("patterns, ...")
- 属性でオブジェクト検索 ("t.c#i[attrName=attrValue, index]")
- HTML記号 &amp 処理
- tidy機能追加、未実装だが beta1.0 で既にあったが。。。


## ライブラリの歴史

- **myhtml（beta1.0.0, 2023年）**
  - フォルダサイズ: 63.5KB
  - 最初のバージョンは myhtml（すべて小文字）として開発され、HTMLパースや検索など完全な機能を持っていた。
  - ただし、ファイルが多数に分割されており、利用時の簡潔さに欠けていた。

- **MyHtml (1.0.0, 2024年)**
  - フォルダサイズ: 29.1KB
  - コードを大幅に簡潔化し、ファイル数も減らして再設計。
  - しかし、機能面では `beta1.0.0` の myhtml よりも検索機能が限定的で、中途半端な部分もたくさん。

- **myhtml2（2.0.0, 2025年）**
  - フォルダサイズ: 58.3KB
  - より進化した設計で、ファイル分割を最小限に抑えつつ、安定性・パフォーマンス・機能性のすべてが過去最高レベルに。
  - 検索やストリーム、libcurl対応など多くの新機能を搭載。

---

## ライセンス

MIT License
