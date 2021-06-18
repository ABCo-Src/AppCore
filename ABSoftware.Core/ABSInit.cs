using ABSoftware.Core.Logging;
using ABSoftware.Core.Registry;
using System;
using System.Collections.Generic;
using System.Data;
using System.IO;
using System.Text;

namespace ABSoftware.Core
{
    public static class ABSInit
    {
        public static void Init(string[] args, string defaultAppSegment, Action createAppSegment)
        {
            LoadAppSegmentName(args, defaultAppSegment);
            InitConfig(createAppSegment);
        }

        static void InitConfig(Action createAppSegment)
        {
            ABSGlobal.ConfigDirectory = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "ABSoftware");
            ABSGlobal.RegistrySegmentsDirectory = Path.Combine(ABSGlobal.ConfigDirectory, "RegistrySegments");

            if (!Directory.Exists(ABSGlobal.ConfigDirectory)) Directory.CreateDirectory(ABSGlobal.ConfigDirectory);
            if (!Directory.Exists(ABSGlobal.RegistrySegmentsDirectory)) Directory.CreateDirectory(ABSGlobal.RegistrySegmentsDirectory);

            if (!ABSRegistry.SegmentExists(ABSGlobal.ABSoftwareSegment)) CreateABSoftwareSegment();
            if (!ABSRegistry.SegmentExists(ABSGlobal.AppSegment)) createAppSegment();
        }

        static void CreateABSoftwareSegment()
        {
            //var newSegment = RegSegment.Create(ABSGlobal.ABSoftwareSegment);

            //ABSRegistry.AddSegment(ABSGlobal.ABSoftwareSegment, newSegment);
        }

        static void LoadAppSegmentName(string[] args, string defaultAppSegment)
        {
            if (args.Length == 0)
            {
                ABSGlobal.AppSegment = defaultAppSegment;
                return;
            }

            if (args.Length < 2 || args[0] != "-appSegment") ABSLog.ThrowError("Unrecognized arguments.");

            
            ABSGlobal.AppSegment = args[1];
        }
    }
}
