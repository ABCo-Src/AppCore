using System;
using System.Collections.Generic;
using System.Text;

namespace ABSoftware.Core.Registry
{
    public interface IRegItem {
        event EventHandler ItemChanged;
    }
}
