using ABSoftware.Core.Logging;
using ABSoftware.Core.Registry;
using ABSoftware.Core.Registry.Items;
using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading;

namespace ABSoftware.Core.ViewModels
{
    internal class RegistrySyncInfo
    {
        internal PropertyInfo Property;
        internal IRegItem Item;
        internal byte BlGroupPos;

        internal RegistrySyncInfo(PropertyInfo property, IRegItem item, byte blGroupPos)
        {
            Property = property;
            Item = item;
            BlGroupPos = blGroupPos;
        }
    }

    /// <summary>
    /// The base for view models in an ABSoftware application.
    /// </summary>
    public class BaseViewModel<T> : INotifyPropertyChanged
    {
        readonly object _registryOperationLock = new object();
        string _suppressPropertyName = null;
        readonly Dictionary<string, RegistrySyncInfo> _registrySyncProperties = new Dictionary<string, RegistrySyncInfo>();

        public event PropertyChangedEventHandler PropertyChanged;

        public virtual void OnPropertyChanged(string propertyName) => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));

        public BaseViewModel()
        {
            var allProperties = typeof(T).GetProperties();
            PropertyChanged += (s, e) => UpdateRegistry(e.PropertyName);

            for (int i = 0; i < allProperties.Length; i++)
            {
                var attribute = allProperties[i].GetCustomAttribute<RegistrySyncAttribute>();
                if (attribute != null)
                {
                    var regItem = ABSRegistry.GetItem(attribute.RegistryPath);
                    if (regItem is RegGroup) ABSLog.ThrowError("Cannot registry sync to a group.");

                    var syncInfo = new RegistrySyncInfo(allProperties[i], regItem, attribute.BooleanGroupPos);
                    _registrySyncProperties.Add(allProperties[i].Name, syncInfo);
                    AddEventHandlerToItem(syncInfo, regItem);

                    UpdateProperty(syncInfo);
                }
            }
        }

        void AddEventHandlerToItem(RegistrySyncInfo info, IRegItem regItem)
        {
            if (regItem is RegBooleanGroup blGr) blGr.BooleanItemChanged += (s, e) =>
            {
                if (info.BlGroupPos == e.Pos)
                    UpdateProperty(info);
            };

            else regItem.ItemChanged += (s, e) => UpdateProperty(info);
        }

        void UpdateProperty(RegistrySyncInfo syncInfo)
        {
            lock (_registryOperationLock)
            {
                if (_suppressPropertyName == syncInfo.Property.Name)
                    return;

                _suppressPropertyName = syncInfo.Property.Name;
                if (syncInfo.Item is RegString str) syncInfo.Property.SetValue(this, str.Data);
                else if (syncInfo.Item is RegInteger integer) syncInfo.Property.SetValue(this, integer.Data);
                else if (syncInfo.Item is RegDecimal dec) syncInfo.Property.SetValue(this, dec.Data);
                else if (syncInfo.Item is RegBooleanGroup blGr) syncInfo.Property.SetValue(this, blGr.GetItem(syncInfo.BlGroupPos));
                _suppressPropertyName = null;
            }
        }

        void UpdateRegistry(string propertyName)
        {
            lock (_registryOperationLock)
            {
                if (_suppressPropertyName == propertyName)
                    return;

                if (_registrySyncProperties.TryGetValue(propertyName, out RegistrySyncInfo syncInfo))
                {
                    var propertyValue = syncInfo.Property.GetValue(this);

                    _suppressPropertyName = propertyName;
                    if (syncInfo.Item is RegString str) str.Data = (string)propertyValue;
                    else if (syncInfo.Item is RegInteger integer) integer.Data = (int)propertyValue;
                    else if (syncInfo.Item is RegDecimal dec) dec.Data = (float)propertyValue;
                    else if (syncInfo.Item is RegBooleanGroup blGr) blGr.SetItem(syncInfo.BlGroupPos, (bool)propertyValue);
                    _suppressPropertyName = null;
                }
            }
        }
    }
}
