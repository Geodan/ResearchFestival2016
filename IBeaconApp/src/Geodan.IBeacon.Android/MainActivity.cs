using System.Linq;
using Android.App;
using Android.OS;
using Android.Widget;
using RadiusNetworks.IBeaconAndroid;
using System;
using Android.Content;

namespace Geodan.IBeacons.Android
{
    [Activity(Label = "Geodan IBeacon Tracker", MainLauncher = true)]
    public class MainActivity : Activity, IBeaconConsumer
    {
        const string UUI = "B9407F30-F5F8-466E-AFF9-25556B57FE6D";
        const int blauwe = 46572;
        const int paarse = 25426;
        const int groene = 29299;

        const string BEACON_ID = "iOSBeacon";
        int datastreamid = 36;
        string name = "Bert";

        IBeaconManager beaconMgr;
        MonitorNotifier monitorNotifier;
        RangeNotifier rangeNotifier;
        Region monitoringRegion;
        Region rangingRegion;
        TextView beaconStatusLabel;
        TextView beaconStatusUpdateTime;

        public MainActivity()
        {
            beaconMgr = IBeaconManager.GetInstanceForApplication(this);

            monitorNotifier = new MonitorNotifier();
            monitoringRegion = new Region(BEACON_ID, UUI, null, null);

            rangeNotifier = new RangeNotifier();
            rangingRegion = new Region(BEACON_ID, UUI, null, null);
        }

        private string GetLocation(int major)
        {
            var res = string.Empty;
            switch (major)
            {
                case blauwe:
                    res = "PK Keuken";
                    break;
                case groene:
                    res = "PK 2e";
                    break;
                case paarse:
                    res = "PK Balie";
                    break;
            }
            return res;
        }

        public void OnIBeaconServiceConnect()
        {
            beaconMgr.SetMonitorNotifier(monitorNotifier);
            beaconMgr.SetRangeNotifier(rangeNotifier);

            beaconMgr.StartMonitoringBeaconsInRegion(monitoringRegion);
            beaconMgr.StartRangingBeaconsInRegion(rangingRegion);
        }

        protected override void OnCreate(Bundle bundle)
        {
            base.OnCreate(bundle);

            SetContentView(Resource.Layout.Main);

            beaconStatusLabel = FindViewById<TextView>(Resource.Id.beaconStatusLabel);
            beaconStatusUpdateTime = FindViewById<TextView>(Resource.Id.beaconStatusUpdateTime);
            var editTextName = FindViewById<TextView>(Resource.Id.editText1);

            var prefs1 = Application.Context.GetSharedPreferences("MyApp1", FileCreationMode.Private);
            var usernameStored = prefs1.GetString("username", null);
            if (!String.IsNullOrEmpty(usernameStored))
            {
                editTextName.Text = usernameStored;
            }

            beaconMgr.Bind(this);

            monitorNotifier.EnterRegionComplete += EnteredRegion;
            monitorNotifier.ExitRegionComplete += ExitedRegion;

            rangeNotifier.DidRangeBeaconsInRegionComplete += RangingBeaconsInRegion;

            Button button = FindViewById<Button>(Resource.Id.button1);
            button.Click += delegate
            {
                name = editTextName.Text;
                var prefs = Application.Context.GetSharedPreferences("MyApp1", FileCreationMode.Private);
                var prefEditor = prefs.Edit();
                prefEditor.PutString("username", name);
                prefEditor.Commit();

                editTextName.ClearFocus();
            };
        }

        void CloseApp()
        {
            base.OnDestroy();

            monitorNotifier.EnterRegionComplete -= EnteredRegion;
            monitorNotifier.ExitRegionComplete -= ExitedRegion;

            rangeNotifier.DidRangeBeaconsInRegionComplete -= RangingBeaconsInRegion;

            beaconMgr.StopMonitoringBeaconsInRegion(monitoringRegion);
            beaconMgr.StopRangingBeaconsInRegion(rangingRegion);
            beaconMgr.UnBind(this);
        }

        void EnteredRegion(object sender, MonitorEventArgs e)
        {
            // ShowMessage("1", "Entered region",null);
        }

        void ExitedRegion(object sender, MonitorEventArgs e)
        {
            ShowMessage("0", "Exit region", "onbekend");
        }

        void RangingBeaconsInRegion(object sender, RangeEventArgs e)
        {
            if (e.Beacons.Count > 0)
            {
                var beacon = e.Beacons.FirstOrDefault();
                var proximity = beacon.Proximity;
                var loc = GetLocation(beacon.Major);
                switch ((ProximityType)beacon.Proximity)
                {
                    case ProximityType.Immediate:
                        ShowMessage("1", "Beacon is immediate", loc);
                        break;
                    case ProximityType.Near:
                        ShowMessage("1", "Beacon is near", loc);
                        break;
                    case ProximityType.Far:
                        ShowMessage("1", "Beacon is far", loc);
                        break;
                    case ProximityType.Unknown:
                        ShowMessage("1", "Beacon is unknown", loc);
                        break;
                }
            }
        }

        void ShowMessage(string message, string description, string location)
        {
            Gost.PostToGost(datastreamid, name + "_" + message + "_" + location);
            RunOnUiThread(() =>
            {
                beaconStatusLabel.Text = message + ": " + description + ", " + location;
                beaconStatusUpdateTime.Text = DateTime.Now.ToString();
            });
        }


        protected override void OnDestroy()
        {
            CloseApp();
        }

        protected override void OnResume()
        {
            base.OnResume();
            if (beaconMgr.IsBound(this))
            {
                beaconMgr.SetBackgroundMode(this, false);
            }
        }

        protected override void OnPause()
        {
            base.OnPause();

            if (beaconMgr.IsBound(this))
            {
                beaconMgr.SetBackgroundMode(this, true);
            }

        }
    }
}