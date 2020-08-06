using System;
using System.Collections.Generic;
using System.Text;

namespace ABSoftware.Core.ViewModels
{
    [AttributeUsage(AttributeTargets.Property)]
    public class RegistrySyncAttribute : Attribute
    {
        public string RegistryPath;
        public byte BooleanGroupPos;

        public RegistrySyncAttribute(string registryPath, byte booleanGroupPos = 0)
        {
            RegistryPath = registryPath;
            BooleanGroupPos = booleanGroupPos;
        }
    }
}
