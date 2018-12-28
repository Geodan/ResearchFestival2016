namespace Geodan.IBeacons.Core
{
    public class GeodanBeacons
    {
        public static string GetLocation(int major)
        {
            var res = string.Empty;
            switch (major)
            {
                case Settings.Blauwe:
                    res = "PK Keuken";
                    break;
                case Settings.Groene:
                    res = "PK 2e";
                    break;
                case Settings.Paarse:
                    res = "PK Balie";
                    break;
            }
            return res;
        }

    }
}
