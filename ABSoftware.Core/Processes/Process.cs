using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace ABSoftware.Core.Processes
{
    /// <summary>
    /// Represents a process that is currently running.
    /// </summary>
    public abstract class Process
    {
        public abstract string ProcessName { get; }

        public bool ProcessIsRunning { get; internal set; }

        /// <summary>
        /// This runs synchornously as the process is queued up.
        /// </summary>
        public virtual void Prepare() { }
        public virtual Task ExecuteAsync() => Task.CompletedTask;
    }
}
