using ABSoftware.Core.Logging;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Net.Http.Headers;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace ABSoftware.Core.Processes
{
    public static class ProcessManager
    {
        // NOTE: These events are NOT thread-safe (hence internal), should mostly only be subscribed to from the UI thread via ABSInit.
        internal static event EventHandler StartProcessing;
        internal static event EventHandler StopProcessing;

        static bool MessageLoopRunning = false;

        static readonly ManualResetEventSlim waitForNext = new ManualResetEventSlim(false);
        internal static ConcurrentQueue<Process> ImmediateProcesses = new ConcurrentQueue<Process>();
        internal static ConcurrentQueue<Process> BackgroundProcesses = new ConcurrentQueue<Process>();

        public static void QueueImmediateProcess(Process process)
        {
            process.Prepare();
            ImmediateProcesses.Enqueue(process);
            waitForNext.Set();
        }

        public static void QueueBackgroundProcess(Process process)
        {
            process.Prepare();
            BackgroundProcesses.Enqueue(process);
            waitForNext.Set();
        }

        internal static void StartMessageLoop()
        {
            if (MessageLoopRunning)
                ABSLog.ThrowError("An attempt to made to start a message loop while one was already running!");
            MessageLoopRunning = true;

            Task.Run(async () =>
            {
                while (true)
                {
                    // Run immediate processes.
                    while (ImmediateProcesses.TryPeek(out Process immediateProcess))
                    {
                        immediateProcess.ProcessIsRunning = true;
                        await immediateProcess.ExecuteAsync();
                        ImmediateProcesses.TryDequeue(out _);
                    }
                        
                    // Run background processes.
                    if (BackgroundProcesses.TryPeek(out Process backgroundProcess))
                    {
                        backgroundProcess.ProcessIsRunning = true;
                        await backgroundProcess.ExecuteAsync();
                        BackgroundProcesses.TryDequeue(out _);
                    }
                    else
                    {
                        StopProcessing?.Invoke(null, null);

                        waitForNext.Wait();

                        if (!MessageLoopRunning)
                            return;

                        waitForNext.Reset();

                        StartProcessing?.Invoke(null, null);
                    }
                }
            });
        }

        internal static void FinishAllAndStop()
        {
            MessageLoopRunning = false;
            waitForNext.Set();
        }

        public static IEnumerator<Process> GetImmediateEnumerator() => ImmediateProcesses.GetEnumerator();
        public static IEnumerator<Process> GetBackgroundEnumerator() => BackgroundProcesses.GetEnumerator();
    }
}
