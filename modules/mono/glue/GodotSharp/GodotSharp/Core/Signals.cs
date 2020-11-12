using System;
using System.Collections.Generic;
using ConnectFlags = Godot.Object.ConnectFlags;
using System.Text.RegularExpressions;
using System.Reflection;

namespace Godot
{
    internal class SignalBackend: IDisposable
    {
        static List<string> Names = new List<string>();
        static Regex Pattern = new Regex("[A-Z]");
        delegate void SignalHandlerDelegate(object target, object[] args);
        SignalHandlerDelegate SignalHandler;
        int NameIndex;
        int FieldIndex;
        internal WeakReference<Godot.Object> Owner;
        bool InternalConnected = false;
        internal List<(object, ConnectFlags)> connections = new List<(object, ConnectFlags)>();
        static string FieldToStringName(string fieldName) {
            return Pattern.Replace(fieldName, "_$0").ToLower().Substring(1);
        }
        public static T FetchValue<T>(object[] args, int idx) {
            if (idx >= args.Length || args[idx] == null) {
                return default(T);
            } else {
                return (T)args[idx];
            }
        }
        public static void HandleSignal(Godot.Object owner, int fieldIndex, object[] args) {
            var signal = owner.GetType().GetFields(BindingFlags.FlattenHierarchy | BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance)[fieldIndex].GetValue(owner) as ISignalInternals;
            signal?.HandleSignal(args);
        }
        public static void InjectTo(Godot.Object pOwner)
        {
            var idx = -1;
            var fields = pOwner.GetType().GetFields(BindingFlags.FlattenHierarchy | BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
            foreach (var field in fields)
            {
                idx++;
                var boxedSignal = field.GetValue(pOwner) as ISignalInternals;
                if (boxedSignal == null || boxedSignal.Backend != null || field.Name.StartsWith("__signal_internal__"))
                {
                    continue;
                }
                var backend = new SignalBackend(pOwner, field, idx);
                boxedSignal.Backend = backend;
                backend.SignalHandler = boxedSignal.HandleSignalForTarget;
                field.SetValue(pOwner, boxedSignal);
            }
        }
        public static void InjectTo(Godot.Object pOwner, PropertyInfo pProp) {
            var signalName = FieldToStringName(pProp.Name);
            var fieldName = "__signal_internal__" + pProp.Name;
            var fields = pOwner.GetType().GetFields(BindingFlags.FlattenHierarchy | BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);
            for (int i = 0; i < fields.Length; i++) {
                var field = fields[i];
                if (field.Name != fieldName) {
                    continue;
                }
                var boxedSignal = field.GetValue(pOwner) as ISignalInternals;
                if (boxedSignal == null || boxedSignal.Backend != null)
                {
                    continue;
                }
                var backend = new SignalBackend(pOwner, signalName, i);
                boxedSignal.Backend = backend;
                backend.SignalHandler = boxedSignal.HandleSignalForTarget;
                field.SetValue(pOwner, boxedSignal);
                break;
            } 
        }
        SignalBackend(Godot.Object pOwner, FieldInfo pField, int pFieldIndex) 
            : this(pOwner, FieldToStringName(pField.Name), pFieldIndex)
        {
        }
        SignalBackend(Godot.Object pOwner, string pSignalName, int pFieldIndex)
        {
            Owner = new WeakReference<Godot.Object>(pOwner, false);
            FieldIndex = pFieldIndex;
            NameIndex = Names.IndexOf(pSignalName);
            if (NameIndex < 0)
            {
                NameIndex = Names.Count;
                Names.Add(pSignalName);
            }
        }
        String SignalName { get { return Names[NameIndex]; } }
        public void Connect(object target, ConnectFlags flags)
        {
            ConnectInternal(target, flags);
            connections.Add((target, flags));
        }

        public void IterateTargets(object[] args)
        {
            var newConnections = new List<(object, ConnectFlags)>();
            foreach (var (target, flags) in connections)
            {
                SignalHandler(target, args);
                if (flags != ConnectFlags.Oneshot)
                {
                    newConnections.Add((target, flags));
                    //Owner.Disconnect(Field.Name, Owner, "HandleSignalInternal");
                }
            }
            connections = newConnections;
            if (connections.Count == 0) {
                DisconnectInternal();
            }
        }
        public void Emit(object[] args)
        {
            Godot.Object owner;
            if (Owner.TryGetTarget(out owner)) owner.EmitSignal(SignalName, args);
        }
        void ConnectInternal(object target, ConnectFlags flags)
        {
            if (!InternalConnected)
            {
                Godot.Object owner;
                if (Owner.TryGetTarget(out owner))
                    owner.Connect(SignalName, owner, "_MonoHandleSignalInternal", new Godot.Collections.Array(new object[] { FieldIndex }), (uint)ConnectFlags.ReferenceCounted);
                InternalConnected = true;
            }
        }

        void DisconnectInternal() {
            Godot.Object owner;
            if (InternalConnected && Owner.TryGetTarget(out owner)) {
                owner.Disconnect(SignalName, owner, "_MonoHandleSignalInternal");
                InternalConnected = false;
            }
        }
        public void Dispose() {
            DisconnectInternal();
        }
    }

    internal interface ISignalInternals
    {
        void HandleSignalForTarget(object target, object[] args);
        void HandleSignal(object[] args);
        SignalBackend Backend { set; get; }
    }

    internal struct SignalBackendReference {
        WeakReference<SignalBackend> Reference;
        public void Set(SignalBackend backend) {
            if (Reference == null) {
                Reference = new WeakReference<SignalBackend>(backend, false);
            } else {
                Reference.SetTarget(backend);
            }
        }
        public SignalBackend Get() {
            SignalBackend backend;
            if (Reference != null && Reference.TryGetTarget(out backend)) {
                return backend;
            } else {
                return null;
            }
        }
    }

    [SignalHandler]
    public struct Signal : IAwaitable, ISignalInternals
    {


        public class Awaiter : IAwaiter
        {
            private bool completed = false;
            
            private Action action;
            public bool IsCompleted => completed;

            public void GetResult()
            {
            }

            public void OnCompleted(Action continuation)
            {
                action = continuation;
            }

            internal void Complete()
            {
                completed = true;
                if (action != null) action();
            }

            internal void Fail()
            {
                action = null;
                completed = true;
            }
        }


        public void Emit()
        {
            _backend.Get()?.Emit(new object[] { });
        }
        public void Connect(Action callback, ConnectFlags flags = 0)
        {
            _backend.Get()?.Connect(callback, flags);
        }
        public IAwaiter GetAwaiter()
        {
            var awaiter = new Awaiter();
            _backend.Get()?.Connect(awaiter, ConnectFlags.Oneshot);
            return awaiter;
        }

        public bool IsValid() {
            return _backend.Get() != null;
        }

        void ISignalInternals.HandleSignalForTarget(object target, object[] args)
        {
            switch (target)
            {
                case Action callback: callback(); break;
                case Awaiter awaiter: awaiter.Complete(); break;
            }
        }

        internal SignalBackendReference _backend;
        SignalBackend ISignalInternals.Backend
        {
            get => _backend.Get();
            set => _backend.Set(value);
        }

        void ISignalInternals.HandleSignal(object[] args)
        {
            _backend.Get()?.IterateTargets(args);
        }
    }

    [SignalHandler]
    public struct Signal<T> : IAwaitable<T>, ISignalInternals
    {
        public class Awaiter : IAwaiter<T>
        {
            private bool completed = false;
            private T result;
            private Action action;
            public bool IsCompleted => completed;

            public T GetResult()
            {
                return result;
            }

            public void OnCompleted(Action continuation)
            {
                action = continuation;
            }

            internal void Complete(T pResult)
            {
                result = pResult;
                completed = true;
                if (action != null) action();
            }

            internal void Fail()
            {
                action = null;
                completed = true;
            }
        }


        public void Emit(T value)
        {
            _backend.Get()?.Emit(new object[] { value });
        }
        public void Connect(Action<T> callback, ConnectFlags flags = 0)
        {
            _backend.Get()?.Connect(callback, flags);
        }
        public IAwaiter<T> GetAwaiter()
        {
            var awaiter = new Awaiter();
            _backend.Get()?.Connect(awaiter, ConnectFlags.Oneshot);
            return awaiter;
        }

        public bool IsValid() {
            return _backend.Get() != null;
        }

        void ISignalInternals.HandleSignalForTarget(object target, object[] args)
        {
            var arg = SignalBackend.FetchValue<T>(args, 0);
            switch (target)
            {
                case Action<T> callback: callback(arg); break;
                case Awaiter awaiter: awaiter.Complete(arg); break;
            }
        }

        internal SignalBackendReference _backend;
        SignalBackend ISignalInternals.Backend
        {
            get => _backend.Get();
            set => _backend.Set(value);
        }

        void ISignalInternals.HandleSignal(object[] args)
        {
            _backend.Get()?.IterateTargets(args);
        }
    }
    [SignalHandler]
    public struct Signal<T0, T1> : IAwaitable<(T0, T1)>, ISignalInternals
    {
        public class Awaiter : IAwaiter<(T0, T1)>
        {
            private bool completed = false;
            private (T0, T1) result;
            private Action action;
            public bool IsCompleted => completed;

            public (T0, T1) GetResult()
            {
                return result;
            }

            public void OnCompleted(Action continuation)
            {
                action = continuation;
            }

            internal void Complete((T0, T1) pResult)
            {
                result = pResult;
                completed = true;
                if (action != null) action();
            }

            internal void Fail()
            {
                action = null;
                completed = true;
            }
        }


        public void Emit(T0 arg0, T1 arg1)
        {
            _backend.Get()?.Emit(new object[] { arg0, arg1 });
        }
        public void Connect(Action<T0, T1> callback, ConnectFlags flags = 0)
        {
            _backend.Get()?.Connect(callback, flags);
        }
        public IAwaiter<(T0, T1)> GetAwaiter()
        {
            var awaiter = new Awaiter();
            _backend.Get()?.Connect(awaiter, ConnectFlags.Oneshot);
            return awaiter;
        }
        public bool IsValid() {
            return _backend.Get() != null;
        }

        void ISignalInternals.HandleSignalForTarget(object target, object[] args)
        {
            var arg0 = SignalBackend.FetchValue<T0>(args, 0);
            var arg1 = SignalBackend.FetchValue<T1>(args, 1);
            
            switch (target)
            {
                case Action<T0, T1> callback: callback(arg0, arg1); break;
                case Awaiter awaiter: awaiter.Complete((arg0, arg1)); break;
            }
        }

        internal SignalBackendReference _backend;
        SignalBackend ISignalInternals.Backend
        {
            get => _backend.Get();
            set => _backend.Set(value);
        }

        void ISignalInternals.HandleSignal(object[] args)
        {
            _backend.Get()?.IterateTargets(args);
        }
    }

    [SignalHandler]
    public struct Signal<T0, T1, T2> : IAwaitable<(T0, T1, T2)>, ISignalInternals
    {
        public class Awaiter : IAwaiter<(T0, T1, T2)>
        {
            private bool completed = false;
            private (T0, T1, T2) result;
            private Action action;
            public bool IsCompleted => completed;

            public (T0, T1, T2) GetResult()
            {
                return result;
            }

            public void OnCompleted(Action continuation)
            {
                action = continuation;
            }

            internal void Complete((T0, T1, T2) pResult)
            {
                result = pResult;
                completed = true;
                if (action != null) action();
            }

            internal void Fail()
            {
                action = null;
                completed = true;
            }
        }


        public void Emit(T0 arg0, T1 arg1, T2 arg2)
        {
            _backend.Get()?.Emit(new object[] { arg0, arg1, arg2 });
        }
        public void Connect(Action<T0, T1, T2> callback, ConnectFlags flags = 0)
        {
            _backend.Get()?.Connect(callback, flags);
        }
        public static Signal<T0, T1, T2> operator +(Signal<T0, T1, T2> signal, Action<T0, T1, T2> action){
            signal.Connect(action);
            return signal;
        }
        public IAwaiter<(T0, T1, T2)> GetAwaiter()
        {
            var awaiter = new Awaiter();
            _backend.Get()?.Connect(awaiter, ConnectFlags.Oneshot);
            return awaiter;
        }

        public bool IsValid() {
            return _backend.Get() != null;
        }

        void ISignalInternals.HandleSignalForTarget(object target, object[] args)
        {
            var arg0 = SignalBackend.FetchValue<T0>(args, 0);
            var arg1 = SignalBackend.FetchValue<T1>(args, 1);
            var arg2 = SignalBackend.FetchValue<T2>(args, 2);
            
            switch (target)
            {
                case Action<T0, T1, T2> callback: callback(arg0, arg1, arg2); break;
                case Awaiter awaiter: awaiter.Complete((arg0, arg1, arg2)); break;
            }
        }

        internal SignalBackendReference _backend;
        SignalBackend ISignalInternals.Backend
        {
            get => _backend.Get();
            set => _backend.Set(value);
        }

        void ISignalInternals.HandleSignal(object[] args)
        {
            _backend.Get()?.IterateTargets(args);
        }
    }

    [SignalHandler]
    public struct Signal<T0, T1, T2, T3> : IAwaitable<(T0, T1, T2, T3)>, ISignalInternals
    {
        public class Awaiter : IAwaiter<(T0, T1, T2, T3)>
        {
            private bool completed = false;
            private (T0, T1, T2, T3) result;
            private Action action;
            public bool IsCompleted => completed;

            public (T0, T1, T2, T3) GetResult()
            {
                return result;
            }

            public void OnCompleted(Action continuation)
            {
                action = continuation;
            }

            internal void Complete((T0, T1, T2, T3) pResult)
            {
                result = pResult;
                completed = true;
                if (action != null) action();
            }

            internal void Fail()
            {
                action = null;
                completed = true;
            }
        }


        public void Emit(T0 arg0, T1 arg1, T2 arg2, T3 arg3)
        {
            _backend.Get()?.Emit(new object[] { arg0, arg1, arg2, arg3 });
        }
        public void Connect(Action<T0, T1, T2, T3> callback, ConnectFlags flags = 0)
        {
            _backend.Get()?.Connect(callback, flags);
        }
        public IAwaiter<(T0, T1, T2, T3)> GetAwaiter()
        {
            var awaiter = new Awaiter();
            _backend.Get()?.Connect(awaiter, ConnectFlags.Oneshot);
            return awaiter;
        }

        public bool IsValid() {
            return _backend.Get() != null;
        }

        void ISignalInternals.HandleSignalForTarget(object target, object[] args)
        {
            var arg0 = SignalBackend.FetchValue<T0>(args, 0);
            var arg1 = SignalBackend.FetchValue<T1>(args, 1);
            var arg2 = SignalBackend.FetchValue<T2>(args, 2);
            var arg3 = SignalBackend.FetchValue<T3>(args, 3);

            switch (target)
            {
                case Action<T0, T1, T2, T3> callback: callback(arg0, arg1, arg2, arg3); break;
                case Awaiter awaiter: awaiter.Complete((arg0, arg1, arg2, arg3)); break;
            }
        }

        internal SignalBackendReference _backend;
        SignalBackend ISignalInternals.Backend
        {
            get => _backend.Get();
            set => _backend.Set(value);
        }

        void ISignalInternals.HandleSignal(object[] args)
        {
            _backend.Get()?.IterateTargets(args);
        }
    }
    
    [SignalHandler]
    public struct Signal<T0, T1, T2, T3, T4> : IAwaitable<(T0, T1, T2, T3, T4)>, ISignalInternals
    {
        public class Awaiter : IAwaiter<(T0, T1, T2, T3, T4)>
        {
            private bool completed = false;
            private (T0, T1, T2, T3, T4) result;
            private Action action;
            public bool IsCompleted => completed;

            public (T0, T1, T2, T3, T4) GetResult()
            {
                return result;
            }

            public void OnCompleted(Action continuation)
            {
                action = continuation;
            }

            internal void Complete((T0, T1, T2, T3, T4) pResult)
            {
                result = pResult;
                completed = true;
                if (action != null) action();
            }

            internal void Fail()
            {
                action = null;
                completed = true;
            }
        }


        public void Emit(T0 arg0, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
        {
            _backend.Get()?.Emit(new object[] { arg0, arg1, arg2, arg3, arg4 });
        }
        public void Connect(Action<T0, T1, T2, T3, T4> callback, ConnectFlags flags = 0)
        {
            _backend.Get()?.Connect(callback, flags);
        }
        public IAwaiter<(T0, T1, T2, T3, T4)> GetAwaiter()
        {
            var awaiter = new Awaiter();
            _backend.Get()?.Connect(awaiter, ConnectFlags.Oneshot);
            return awaiter;
        }

        public bool IsValid() {
            return _backend.Get() != null;
        }

        void ISignalInternals.HandleSignalForTarget(object target, object[] args)
        {
            var arg0 = SignalBackend.FetchValue<T0>(args, 0);
            var arg1 = SignalBackend.FetchValue<T1>(args, 1);
            var arg2 = SignalBackend.FetchValue<T2>(args, 2);
            var arg3 = SignalBackend.FetchValue<T3>(args, 3);
            var arg4 = SignalBackend.FetchValue<T4>(args, 4);
            switch (target)
            {
                case Action<T0, T1, T2, T3, T4> callback: callback(arg0, arg1, arg2, arg3, arg4); break;
                case Awaiter awaiter: awaiter.Complete((arg0, arg1, arg2, arg3, arg4)); break;
            }
        }

        internal SignalBackendReference _backend;
        SignalBackend ISignalInternals.Backend
        {
            get => _backend.Get();
            set => _backend.Set(value);
        }

        void ISignalInternals.HandleSignal(object[] args)
        {
            _backend.Get()?.IterateTargets(args);
        }
    }
}