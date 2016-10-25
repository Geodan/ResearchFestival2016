using System;
using CoreGraphics;
using Foundation;
using UIKit;
using CoreBluetooth;
using CoreLocation;
using CoreFoundation;

namespace iOSBeacon
{
	public partial class iOSBeaconViewController : UIViewController
	{
		CBPeripheralManager peripheralMgr;
		BTPeripheralDelegate peripheralDelegate;

		public iOSBeaconViewController () : base ("iOSBeaconViewController", null)
		{
		}

		public override void ViewDidLoad ()
		{
			base.ViewDidLoad ();

			var uuid = new NSUuid ("A1F30FF0-0A9F-4DE0-90DA-95F88164942E");
			var beaconId = "iOSBeacon";
			var beaconRegion = new CLBeaconRegion (uuid, beaconId) {
				NotifyEntryStateOnDisplay = true,
				NotifyOnEntry = true,
				NotifyOnExit = true
			};

			var peripheralData = beaconRegion.GetPeripheralData (new NSNumber (-59));

			peripheralDelegate = new BTPeripheralDelegate ();
			peripheralMgr = new CBPeripheralManager (peripheralDelegate, DispatchQueue.DefaultGlobalQueue);
			peripheralMgr.StartAdvertising (peripheralData);
		}

		class BTPeripheralDelegate : CBPeripheralManagerDelegate
		{
			public override void StateUpdated (CBPeripheralManager peripheral)
			{
				if (peripheral.State == CBPeripheralManagerState.PoweredOn) {
					Console.WriteLine ("iBeacon powered on");
				}
			}
		}
	}
}