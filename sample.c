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
  HtmlObject* tagBody = HtmlFindObject(doc, "body[-1]" /* インデックス -1 に設定することで逆方向の検索ができます */);

  // GetObjectInnerText() と違って、GetObjectText() は子オブジェクトのテキストも含めたゲットなので新しいバッファが必要
  printf("Text:\n%s\n", HtmlGetObjectText(tagBody));

  /* (オプション) HTML 出力 */
  HtmlWriteObjectToFile(doc, "output.html");

  /* クリーンアップ */
  HtmlDestroyObject(doc);

  curl_easy_cleanup(curl);
  return 0;
}