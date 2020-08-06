using ABSoftware.Core.Processes;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Text;

namespace ABSoftware.Core.UnitTests.Processes
{
    [TestClass]
    public class ProcessesTests
    {
        [TestMethod]
        public void QueueProcesses()
        {
            List<int> intList = new List<int>();

            ProcessManager.StopProcessing += (s, e) =>
            {
                CollectionAssert.AreEqual(new int[0, 2, 1], intList);
            };

            ProcessManager.QueueImmediateProcess(new ActionProcess(() => intList.Add(0)));
            ProcessManager.QueueBackgroundProcess(new ActionProcess(() => intList.Add(1)));
            ProcessManager.QueueImmediateProcess(new ActionProcess(() => intList.Add(2)));

            ProcessManager.StartMessageLoop();
        }
    }
}
