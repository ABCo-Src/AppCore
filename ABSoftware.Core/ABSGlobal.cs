using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace ABSoftware.Core
{
    public static class ABSGlobal
    {
        public static string ConfigDirectory { get; internal set; }
        public static string RegistrySegmentsDirectory { get; internal set; }
        public static string AppSegment { get; set; }
        public static string ABSoftwareSegment { get; set; } = "absoftware";
    }
}
