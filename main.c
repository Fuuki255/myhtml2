#include <curl/curl.h>        // libcurl の拡張機能を使用するには myhtml.h 前に定義する必要がある
#include "myhtml2/myhtml.h"

int main(int argc, char** argv) {
  // libcurl 初期化
  CURL* curl = curl_easy_init();

  // HTML 取得
  HtmlObject* doc = HtmlReadObjectFromCURL(curl, "https://example.com");

  // 検索1 : title
  HtmlObject* tagTitle = HtmlFindObject(doc, "title");
  printf("HTML %s\n", HtmlGetObjectInnerText(tagTitle));

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
  putchar('\n');
  
  HtmlDestroySelect(&select);   // HtmlSelect の使用後は削除する必要があります

  // 検索5 : マルチ検索2
  HtmlArray array = HtmlFindAllObjects(doc, "p", 8);     // HtmlSelect の結果を配列にまとめるメソッド、 出力の HtmlArray は同様に削除する必要がある
  
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