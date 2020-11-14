using System;

namespace Godot
{
    [AttributeUsage(AttributeTargets.Struct|AttributeTargets.Field)]
    internal class ManagedSignalAttribute : Attribute
    {
    }
}