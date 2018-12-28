using System.Linq;
using Android.App;
using Android.OS;
using Android.Widget;
using RadiusNetworks.IBeaconAndroid;
using System;
using Android.Content;
using Android.Bluetooth;
using Geodan.IBeacons.Core;
using Android.Util;

namespace Geodan.IBeacons.Android
{
    [Activity(Label = "Geodan IBeacon Tracker 0.1", MainLauncher = true)]
    public class MainActivity : Activity, IBeaconConsumer
    {

        const string BEACON_ID = "iOSBeacon";

        IBeaconManager beaconMgr;
        RangeNotifier rangeNotifier;
        MonitorNotifier monitorNotifier;
        Region rangingRegion;
        Region monitoringRegion;
        TextView beaconStatusLabel;
        TextView beaconStatusUpdateTime;
        string lastKnownRegion = null;

        public MainActivity()
        {
            Log.Info("geodan main", "startup");
            BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.DefaultAdapter;
            if (mBluetoothAdapter == null)
            {
                RunOnUiThread(() =>
                {
                    showAlert("Bluetooth", "No bluetooth available");
                });
            }
            else
            {
                if (!mBluetoothAdapter.IsEnabled)
                {
                    RunOnUiThread(() =>
                    {
                        showAlert("Bluetooth", "Please activate bluetooth");
                    });
                }
            }
            beaconMgr = IBeaconManager.GetInstanceForApplication(this);

            monitorNotifier = new MonitorNotifier();
            monitoringRegion = new Region(BEACON_ID, Settings.UUI, null, null);

            rangeNotifier = new RangeNotifier();
            rangingRegion = new Region(BEACON_ID, Settings.UUI, null, null);
        }

        private void showAlert(string Tile, string Message)
        {
            var alert = new AlertDialog.Builder(this);
            alert.SetTitle(Title);
            alert.SetMessage(Message);
            alert.SetPositiveButton("Ok", (senderAlert, args) => {
                Finish();
            });
            var dialog = alert.Create();
            dialog.Show();
        }

        public void OnIBeaconServiceConnect()
        {
            beaconMgr.SetRangeNotifier(rangeNotifier);
            beaconMgr.StartRangingBeaconsInRegion(rangingRegion);
        }

        protected override void OnCreate(Bundle bundle)
        {
            Log.Info("geodan main", "OnCreate");

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

            var button = FindViewById<Button>(Resource.Id.button1);
            button.Click += delegate
            {
                Settings.name = editTextName.Text;
                var prefs = Application.Context.GetSharedPreferences("MyApp1", FileCreationMode.Private);
                var prefEditor = prefs.Edit();
                prefEditor.PutString("username", Settings.name);
                prefEditor.Commit();

                editTextName.ClearFocus();
            };
        }

        void CloseApp()
        {
            rangeNotifier.DidRangeBeaconsInRegionComplete -= RangingBeaconsInRegion;
            beaconMgr.StopRangingBeaconsInRegion(rangingRegion);
            monitorNotifier.EnterRegionComplete -= EnteredRegion;
            monitorNotifier.ExitRegionComplete -= ExitedRegion;

            beaconMgr.UnBind(this);
            base.OnDestroy();
        }


        void EnteredRegion(object sender, MonitorEventArgs e)
        {
            // ShowMessage("1", "Entered region",null);
        }

        void ExitedRegion(object sender, MonitorEventArgs e)
        {
            if (String.IsNullOrEmpty(lastKnownRegion))
            {
                ShowMessage("0", "Exit region", "onbekend");
            }
            else
            {
                ShowMessage("0", "Exit region", lastKnownRegion);
            }
        }


        void RangingBeaconsInRegion(object sender, RangeEventArgs e)
        {
            Log.Info("geodan main", "RangingBeaconsInMain called");

            if (e.Beacons.Count > 0)
            {
                Log.Info("geodan main", "Number of beacons: "+ e.Beacons.Count);

                var beacon = e.Beacons.FirstOrDefault();
                var proximity = beacon.Proximity;
                var loc = GeodanBeacons.GetLocation(beacon.Major);
                lastKnownRegion = loc;
                switch ((ProximityType)beacon.Proximity)
                {
                    case ProximityType.Immediate:
                        ShowMessage("1", "Beacon is immediate (cm's)", loc);
                        break;
                    case ProximityType.Near:
                        ShowMessage("1", "Beacon is near (meters)", loc);
                        break;
                    case ProximityType.Far:
                        ShowMessage("1", "Beacon is far (few meters)", loc);
                        break;
                    case ProximityType.Unknown:
                        ShowMessage("1", "Beacon is unknown", loc);
                        break;
                }
            }
        }

        void ShowMessage(string message, string description, string location)
        {
            Log.Info("geodan main", $"Post to GOST: " + Settings.name + "_" + message + "_" + location);

            try
            {
                Gost.PostToGost(Settings.datastreamid, Settings.name + "_" + message + "_" + location);
            }
            catch(Exception ex)
            {
                message += " (Post error)";
                Log.Info("geodan main", "Fout bij posten: " + ex.ToString());
            }
            RunOnUiThread(() =>
            {
                beaconStatusLabel.Text = message + ": " + description + ", " + location;
                beaconStatusUpdateTime.Text = DateTime.Now.ToString();
            });
        }

        protected override void OnDestroy()
        {
            Log.Info("geodan main", "Close App called");

            CloseApp();
        }
    }
}