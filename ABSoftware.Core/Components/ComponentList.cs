using System;
using System.Collections.Generic;
using System.Text;

namespace ABSoftware.Core.Components
{
    public class ComponentList
    {
        public event ComponentListUpdatedEventHandler Updated;

        readonly List<IComponent> _components = new List<IComponent>();

        public ComponentList() => _components = new List<IComponent>();
        public ComponentList(int capacity) => _components = new List<IComponent>(capacity);

        public void RegisterComponent(IComponent component)
        {
            _components.Add(component);
            Updated?.Invoke(this, new ComponentListUpdatedEventArgs(component));
        }

        public void UnregisterComponent(int componentIndex)
        {
            _components.RemoveAt(componentIndex);
            Updated?.Invoke(this, new ComponentListUpdatedEventArgs(componentIndex));
        }

        public void ReregisterComponent(IComponent newComponent, int index)
        {
            _components[index] = newComponent;
            Updated?.Invoke(this, new ComponentListUpdatedEventArgs(newComponent, index));
        }

        public int Count => _components.Count;
        public IEnumerator<IComponent> GetEnumerator() => _components.GetEnumerator();
    }

    public enum ComponentListUpdatedType
    {
        Register,
        Unregister,
        Reregister
    }

    public class ComponentListUpdatedEventArgs : EventArgs
    {
        public ComponentListUpdatedType Type;
        public IComponent Registered;
        public int Index;

        public ComponentListUpdatedEventArgs(IComponent registered)
        {
            Type = ComponentListUpdatedType.Register;
            Registered = registered;
        }

        public ComponentListUpdatedEventArgs(int index)
        {
            Type = ComponentListUpdatedType.Unregister;
            Index = index;
        }

        public ComponentListUpdatedEventArgs(IComponent registered, int itemRemoved)
        {
            Type = ComponentListUpdatedType.Reregister;
            Registered = registered;
            Index = itemRemoved;
        }
    }

    public delegate void ComponentListUpdatedEventHandler(object sender, ComponentListUpdatedEventArgs e);
}
