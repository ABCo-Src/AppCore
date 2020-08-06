using ABSoftware.Core.Components;
using System;
using System.Collections.Generic;
using System.Text;

namespace ABSoftware.Core.UnitTests.Components
{
    public class TestComponent : IComponent
    {
        public string Title => "Test Component";

        public string Description => "Tests the component system.";
    }
}
