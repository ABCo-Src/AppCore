using System;
using System.Collections.Generic;
using System.Text;

namespace ABSoftware.Core.Logging
{
    public static class ABSLog
    {
        public static Dictionary<string, int> RegisteredLogMessages = new Dictionary<string, int>();

        public static void ThrowError(string message)
        {
            throw new Exception(message);
        }

        public static void TriggerWarning(string message)
        {
            // Do something.
        }
    }
}
