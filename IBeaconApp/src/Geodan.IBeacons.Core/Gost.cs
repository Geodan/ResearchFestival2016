using System.Dynamic;
using System.Net.Http;
using System.Net.Http.Headers;

namespace Geodan.IBeacons.Android
{

    public static class Gost
    {
        public static void PostToGost(int datastreamid, string Message)
        {
            using (var client = new HttpClient())
            {
                client.BaseAddress = new System.Uri("http://gost.geodan.nl");
                client.DefaultRequestHeaders.Accept.Clear();

                dynamic dynamicJson = new ExpandoObject();
                dynamicJson.result = Message;
                var json = Newtonsoft.Json.JsonConvert.SerializeObject(dynamicJson);

                var postString = new StringContent(json);
                postString.Headers.ContentType = new MediaTypeHeaderValue("application/json");
                var result = client.PostAsync("v1.0/Datastreams(" + datastreamid + ")/Observations", postString).Result;
                string resultContent = result.Content.ReadAsStringAsync().Result;
            }

        }
    }
}