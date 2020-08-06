using System;
using System.Collections.Generic;
using System.Text;

namespace ABSoftware.Core.Components
{
    public interface IComponent
    {
        string Title { get; }
        string Description { get; }
    }
}
