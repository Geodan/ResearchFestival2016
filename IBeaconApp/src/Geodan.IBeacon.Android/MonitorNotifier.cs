using System;
using RadiusNetworks.IBeaconAndroid;

namespace Geodan.IBeacons.Android
{
	public class MonitorEventArgs : EventArgs
	{
		public Region Region { get; set; }

		public int State { get; set; }
	}

	public class MonitorNotifier : Java.Lang.Object, IMonitorNotifier
	{
		public event EventHandler<MonitorEventArgs> DetermineStateForRegionComplete;
		public event EventHandler<MonitorEventArgs> EnterRegionComplete;
		public event EventHandler<MonitorEventArgs> ExitRegionComplete;

		public void DidDetermineStateForRegion (int p0, Region p1)
		{
			OnDetermineStateForRegionComplete ();
		}

		public void DidEnterRegion (Region p0)
		{
			OnEnterRegionComplete ();
		}

		public void DidExitRegion (Region p0)
		{
			OnExitRegionComplete ();
		}

		void OnDetermineStateForRegionComplete ()
		{
            DetermineStateForRegionComplete?.Invoke(this, new MonitorEventArgs());
        }

		void OnEnterRegionComplete ()
		{
            EnterRegionComplete?.Invoke(this, new MonitorEventArgs());
        }

		void OnExitRegionComplete ()
		{
            ExitRegionComplete?.Invoke(this, new MonitorEventArgs());
        }
	}
}