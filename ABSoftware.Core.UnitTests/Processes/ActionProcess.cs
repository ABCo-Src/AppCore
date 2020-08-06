using ABSoftware.Core.Processes;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace ABSoftware.Core.UnitTests.Processes
{
    public class ActionProcess : Process
    {
        public Action ToRun;

        public override string ProcessName => "Action Running Process";

        public override Task ExecuteAsync()
        {
            ToRun();
            return Task.CompletedTask;
        }

        public ActionProcess(Action toRun) => ToRun = toRun;
    }
}
