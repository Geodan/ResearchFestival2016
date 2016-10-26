using CoreBluetooth;
using CoreFoundation;
using CoreLocation;
using Foundation;
using Geodan.IBeacons.Android;
using Geodan.IBeacons.Core;
using System;
using System.Threading;
using UIKit;

namespace Geodan.IBeacons.IPhone
{
    public partial class ViewController : UIViewController
    {
        CBPeripheralManager peripheralMgr;
        BTPeripheralDelegate peripheralDelegate;

        string lastKnownRegion = null;

        public ViewController(IntPtr handle) : base(handle)
        {
            peripheralDelegate = new BTPeripheralDelegate();
            peripheralMgr = new CBPeripheralManager(peripheralDelegate, DispatchQueue.DefaultGlobalQueue);
        }


        private void showAlert(string Title, string Message)
        {
            var _error = new UIAlertView(Title, Message, null, "Ok", null);
            // _error.Clicked += (sender, buttonArgs) => Thread.CurrentThread.Abort(); ;
            _error.Show();
        }

        public override void ViewDidLoad()
        {
            base.ViewDidLoad();
            var defaults = NSUserDefaults.StandardUserDefaults;

            btnName.TouchUpInside += delegate {
                defaults.SetString(lblName.Text, "username");
                Settings.name = lblName.Text;
            };

            if (peripheralMgr.State == CBPeripheralManagerState.Unsupported)
            {
                showAlert("Bluetooth", "No bluetooth support");
            }
            if (peripheralMgr.State == CBPeripheralManagerState.PoweredOff)
            {
                showAlert("Bluetooth", "Please activate bluetooth");
            }

            lblStatus.Text = "loaded!";
            lblTime.Text = DateTime.Now.ToString();
            var username = defaults.StringForKey("username");
            var usernameStored = defaults.StringForKey("username");

            if (!String.IsNullOrEmpty(usernameStored))
            {
                lblName.Text = usernameStored;
                lblName.ResignFirstResponder();
                Settings.name = usernameStored;
            }


            var uuid = new NSUuid(Settings.UUI);
            var beaconId = "iOSBeacon";
            var beaconRegion = new CLBeaconRegion(uuid, beaconId)
            {
                NotifyEntryStateOnDisplay = true,
                NotifyOnEntry = true,
                NotifyOnExit = true
            };

            var locationMgr = new CLLocationManager();
            locationMgr.RequestWhenInUseAuthorization();

            // eurghh http://stackoverflow.com/questions/20124443/ibeacon-get-major-and-minor-only-looking-for-uuid

            locationMgr.RegionEntered += (object sender, CLRegionEventArgs e) =>
            {
                // have to check for e.Region.Identifier?
                // for now do nothing
            };

            locationMgr.DidRangeBeacons += (object sender, CLRegionBeaconsRangedEventArgs e) =>
            {
                if (e.Beacons.Length > 0)
                {
                    var beacon = e.Beacons[0];
                    var loc = GeodanBeacons.GetLocation(beacon.Major.Int32Value);
                    lastKnownRegion = loc;

                    switch (beacon.Proximity)
                    {
                        case CLProximity.Immediate:
                            ShowMessage("1", "Beacon is immediate", loc);
                            break;
                        case CLProximity.Near:
                            ShowMessage("1", "Beacon is near", loc);
                            break;
                        case CLProximity.Far:
                            ShowMessage("1", "Beacon is far", loc);
                            break;
                        case CLProximity.Unknown:
                            ShowMessage("1", "Beacon is unknown", loc);
                            break;
                    }
                }
            };

            locationMgr.RegionLeft += (object sender, CLRegionEventArgs e) =>
            {
                if (String.IsNullOrEmpty(lastKnownRegion))
                {
                    ShowMessage("0", "Exit region", "onbekend");
                }
                else
                {
                    ShowMessage("0", "Exit region", lastKnownRegion);
                }
            };

        }

        void ShowMessage(string message, string description, string location)
        {
            Gost.PostToGost(Settings.datastreamid, Settings.name + "_" + message + "_" + location);

            lblStatus.Text = message + ": " + description + ", " + location;
            lblTime.Text = DateTime.Now.ToString();
        }


        public override void DidReceiveMemoryWarning()
        {
            base.DidReceiveMemoryWarning();
            // Release any cached data, images, etc that aren't in use.
        }
    }

    class BTPeripheralDelegate : CBPeripheralManagerDelegate
    {
        public override void StateUpdated(CBPeripheralManager peripheral)
        {
            if (peripheral.State == CBPeripheralManagerState.PoweredOn)
            {
                Console.WriteLine("powered on");
            }
        }
    }

}