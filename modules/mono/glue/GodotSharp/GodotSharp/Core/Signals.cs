using System;
using ConnectFlags = Godot.Object.ConnectFlags;

namespace Godot
{
    internal interface ISignalField
    {
        void ProcessCallback(object callback, object[] args);
        void CancelCallback(object callback);
        SignalProcessor Processor { set; get; }
    }

    internal struct SignalProcessorReference {
        WeakReference<SignalProcessor> Reference;
        public void Set(SignalProcessor processor) {
            if (Reference == null) {
                Reference = new WeakReference<SignalProcessor>(processor, false);
            } else {
                Reference.SetTarget(processor);
            }
        }
        public SignalProcessor Get() {
            SignalProcessor processor;
            if (Reference != null && Reference.TryGetTarget(out processor)) {
                return processor;
            } else {
                return null;
            }
        }
    }

    [ManagedSignal]
    public struct Signal : IAwaitable, ISignalField
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
            _processor.Get()?.Emit(new object[] { });
        }
        public void Connect(Action callback, ConnectFlags flags = 0)
        {
            _processor.Get()?.Connect(callback, flags);
        }
        public void Connect<TResult>(Func<TResult> callback, ConnectFlags flags = 0)
        {
            _processor.Get()?.Connect(callback, flags);
        }
        public void Disonnect(Action callback)
        {
            _processor.Get()?.Disconnect(callback);
        }
        public void Disconnect<TResult>(Func<TResult> callback)
        {
            _processor.Get()?.Disconnect(callback);
        }
        public void DisconnectAll() {
            _processor.Get()?.DisconnectAll();
        }
        public IAwaiter GetAwaiter()
        {
            var awaiter = new Awaiter();
            _processor.Get()?.Connect(awaiter, ConnectFlags.Oneshot, false);
            return awaiter;
        }

        public bool IsValid() {
            return _processor.Get() != null;
        }
        internal void ProcessCallback(object callback, object[] args) {
            switch (callback)
            {
                case Action action: action(); break;
                case Awaiter awaiter: awaiter.Complete(); break;
                case Delegate func: func.DynamicInvoke(); break;
            }
        }
        void ISignalField.ProcessCallback(object callback, object[] args)
        {
            ProcessCallback(callback, args);
        }
        void ISignalField.CancelCallback(object callback) {
            switch (callback)
            {
                case Awaiter awaiter: awaiter.Fail(); break;
            }
        }

        

        internal SignalProcessorReference _processor;
        SignalProcessor ISignalField.Processor
        {
            get => _processor.Get();
            set => _processor.Set(value);
        }
    }

    [ManagedSignal]
    public struct Signal<T> : IAwaitable<T>, ISignalField
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
            _processor.Get()?.Emit(new object[] { value });
        }
        public void Connect(Action<T> callback, ConnectFlags flags = 0)
        {
            _processor.Get()?.Connect(callback, flags);
        }
        public void Connect<TResult>(Func<T, TResult> callback, ConnectFlags flags = 0)
        {
            _processor.Get()?.Connect(callback, flags);
        }
        public void Disonnect(Action<T> callback)
        {
            _processor.Get()?.Disconnect(callback);
        }
        public void Disconnect<TResult>(Func<T, TResult> callback)
        {
            _processor.Get()?.Disconnect(callback);
        }
        public void DisconnectAll() {
            _processor.Get()?.DisconnectAll();
        }
        public IAwaiter<T> GetAwaiter()
        {
            var awaiter = new Awaiter();
            _processor.Get()?.Connect(awaiter, ConnectFlags.Oneshot, false);
            return awaiter;
        }

        public bool IsValid() {
            return _processor.Get() != null;
        }
        internal void ProcessCallback(object callback, object[] args)
        {
            var arg = SignalProcessor.FetchValue<T>(args, 0);
            switch (callback)
            {
                case Action<T> action: action(arg); break;
                case Awaiter awaiter: awaiter.Complete(arg); break;
                case Delegate func: func.DynamicInvoke(arg); break;
            }
        }
        void ISignalField.ProcessCallback(object callback, object[] args)
        {
            ProcessCallback(callback, args);
        }
        void ISignalField.CancelCallback(object callback) {
            switch (callback)
            {
                case Awaiter awaiter: awaiter.Fail(); break;
            }
        }
        internal SignalProcessorReference _processor;
        SignalProcessor ISignalField.Processor
        {
            get => _processor.Get();
            set => _processor.Set(value);
        }
    }
    [ManagedSignal]
    public struct Signal<T0, T1> : IAwaitable<(T0, T1)>, ISignalField
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
            _processor.Get()?.Emit(new object[] { arg0, arg1 });
        }
        public void Connect(Action<T0, T1> callback, ConnectFlags flags = 0)
        {
            _processor.Get()?.Connect(callback, flags);
        }
        public void Connect<TResult>(Func<T0, T1, TResult> callback, ConnectFlags flags = 0)
        {
            _processor.Get()?.Connect(callback, flags);
        }
        public void Disonnect(Action<T0, T1> callback)
        {
            _processor.Get()?.Disconnect(callback);
        }
        public void Disconnect<TResult>(Func<T0, T1, TResult> callback)
        {
            _processor.Get()?.Disconnect(callback);
        }
        public void DisconnectAll() {
            _processor.Get()?.DisconnectAll();
        }
        public IAwaiter<(T0, T1)> GetAwaiter()
        {
            var awaiter = new Awaiter();
            _processor.Get()?.Connect(awaiter, ConnectFlags.Oneshot, false);
            return awaiter;
        }
        public bool IsValid() {
            return _processor.Get() != null;
        }
        internal void ProcessCallback(object callback, object[] args) {
            var arg0 = SignalProcessor.FetchValue<T0>(args, 0);
            var arg1 = SignalProcessor.FetchValue<T1>(args, 1);
            
            switch (callback)
            {
                case Action<T0, T1> action: action(arg0, arg1); break;
                case Awaiter awaiter: awaiter.Complete((arg0, arg1)); break;
                case Delegate func: func.DynamicInvoke(arg0, arg1); break;
            }
        }
        void ISignalField.ProcessCallback(object callback, object[] args)
        {
            ProcessCallback(callback, args);
        }
        void ISignalField.CancelCallback(object callback) {
            switch (callback)
            {
                case Awaiter awaiter: awaiter.Fail(); break;
            }
        }
        internal SignalProcessorReference _processor;
        SignalProcessor ISignalField.Processor
        {
            get => _processor.Get();
            set => _processor.Set(value);
        }
    }

    [ManagedSignal]
    public struct Signal<T0, T1, T2> : IAwaitable<(T0, T1, T2)>, ISignalField
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
            _processor.Get()?.Emit(new object[] { arg0, arg1, arg2 });
        }
        public void Connect(Action<T0, T1, T2> callback, ConnectFlags flags = 0)
        {
            _processor.Get()?.Connect(callback, flags);
        }
        public void Connect<TResult>(Func<T0, T1, T2, TResult> callback, ConnectFlags flags = 0)
        {
            _processor.Get()?.Connect(callback, flags);
        }
        public void Disonnect(Action<T0, T1, T2> callback)
        {
            _processor.Get()?.Disconnect(callback);
        }
        public void Disconnect<TResult>(Func<T0, T1, T2, TResult> callback)
        {
            _processor.Get()?.Disconnect(callback);
        }
        public void DisconnectAll() {
            _processor.Get()?.DisconnectAll();
        }
        public IAwaiter<(T0, T1, T2)> GetAwaiter()
        {
            var awaiter = new Awaiter();
            _processor.Get()?.Connect(awaiter, ConnectFlags.Oneshot, false);
            return awaiter;
        }

        public bool IsValid() {
            return _processor.Get() != null;
        }
        internal void ProcessCallback(object callback, object[] args) {
            var arg0 = SignalProcessor.FetchValue<T0>(args, 0);
            var arg1 = SignalProcessor.FetchValue<T1>(args, 1);
            var arg2 = SignalProcessor.FetchValue<T2>(args, 2);
            
            switch (callback)
            {
                case Action<T0, T1, T2> action: action(arg0, arg1, arg2); break;
                case Awaiter awaiter: awaiter.Complete((arg0, arg1, arg2)); break;
                case Delegate func: func.DynamicInvoke(arg0, arg1, arg2); break;
            }
        }
        void ISignalField.ProcessCallback(object callback, object[] args)
        {
            ProcessCallback(callback, args);
        }
        void ISignalField.CancelCallback(object callback) {
            switch (callback)
            {
                case Awaiter awaiter: awaiter.Fail(); break;
            }
        }
        internal SignalProcessorReference _processor;
        SignalProcessor ISignalField.Processor
        {
            get => _processor.Get();
            set => _processor.Set(value);
        }
    }

    [ManagedSignal]
    public struct Signal<T0, T1, T2, T3> : IAwaitable<(T0, T1, T2, T3)>, ISignalField
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
            _processor.Get()?.Emit(new object[] { arg0, arg1, arg2, arg3 });
        }
        public void Connect(Action<T0, T1, T2, T3> callback, ConnectFlags flags = 0)
        {
            _processor.Get()?.Connect(callback, flags);
        }
        public void Connect<TResult>(Func<T0, T1, T2, T3, TResult> callback, ConnectFlags flags = 0)
        {
            _processor.Get()?.Connect(callback, flags);
        }
        public void Disonnect(Action<T0, T1, T2, T3> callback)
        {
            _processor.Get()?.Disconnect(callback);
        }
        public void Disconnect<TResult>(Func<T0, T1, T2, T3, TResult> callback)
        {
            _processor.Get()?.Disconnect(callback);
        }
        public void DisconnectAll() {
            _processor.Get()?.DisconnectAll();
        }
        public IAwaiter<(T0, T1, T2, T3)> GetAwaiter()
        {
            var awaiter = new Awaiter();
            _processor.Get()?.Connect(awaiter, ConnectFlags.Oneshot, false);
            return awaiter;
        }

        public bool IsValid() {
            return _processor.Get() != null;
        }
        internal void ProcessCallback(object callback, object[] args)
        {
            var arg0 = SignalProcessor.FetchValue<T0>(args, 0);
            var arg1 = SignalProcessor.FetchValue<T1>(args, 1);
            var arg2 = SignalProcessor.FetchValue<T2>(args, 2);
            var arg3 = SignalProcessor.FetchValue<T3>(args, 3);

            switch (callback)
            {
                case Action<T0, T1, T2, T3> action: action(arg0, arg1, arg2, arg3); break;
                case Awaiter awaiter: awaiter.Complete((arg0, arg1, arg2, arg3)); break;
                case Delegate func: func.DynamicInvoke(arg0, arg1, arg2, arg3); break;
            }
        }
        void ISignalField.ProcessCallback(object callback, object[] args)
        {
            ProcessCallback(callback, args);
        }
        void ISignalField.CancelCallback(object callback) {
            switch (callback)
            {
                case Awaiter awaiter: awaiter.Fail(); break;
            }
        }
        internal SignalProcessorReference _processor;
        SignalProcessor ISignalField.Processor
        {
            get => _processor.Get();
            set => _processor.Set(value);
        }
    }
    
    [ManagedSignal]
    public struct Signal<T0, T1, T2, T3, T4> : IAwaitable<(T0, T1, T2, T3, T4)>, ISignalField
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
            _processor.Get()?.Emit(new object[] { arg0, arg1, arg2, arg3, arg4 });
        }
        public void Connect(Action<T0, T1, T2, T3, T4> callback, ConnectFlags flags = 0)
        {
            _processor.Get()?.Connect(callback, flags);
        }
        public void Connect<TResult>(Func<T0, T1, T2, T3, T4, TResult> callback, ConnectFlags flags = 0)
        {
            _processor.Get()?.Connect(callback, flags);
        }
        public void Disonnect(Action<T0, T1, T2, T3, T4> callback)
        {
            _processor.Get()?.Disconnect(callback);
        }
        public void Disconnect<TResult>(Func<T0, T1, T2, T3, T4, TResult> callback)
        {
            _processor.Get()?.Disconnect(callback);
        }
        public void DisconnectAll() {
            _processor.Get()?.DisconnectAll();
        }
        public IAwaiter<(T0, T1, T2, T3, T4)> GetAwaiter()
        {
            var awaiter = new Awaiter();
            _processor.Get()?.Connect(awaiter, ConnectFlags.Oneshot, false);
            return awaiter;
        }

        public bool IsValid() {
            return _processor.Get() != null;
        }
        internal void ProcessCallback(object callback, object[] args)
        {
            var arg0 = SignalProcessor.FetchValue<T0>(args, 0);
            var arg1 = SignalProcessor.FetchValue<T1>(args, 1);
            var arg2 = SignalProcessor.FetchValue<T2>(args, 2);
            var arg3 = SignalProcessor.FetchValue<T3>(args, 3);
            var arg4 = SignalProcessor.FetchValue<T4>(args, 4);
            switch (callback)
            {
                case Action<T0, T1, T2, T3, T4> action: action(arg0, arg1, arg2, arg3, arg4); break;
                case Awaiter awaiter: awaiter.Complete((arg0, arg1, arg2, arg3, arg4)); break;
                case Delegate func: func.DynamicInvoke(arg0, arg1, arg2, arg3, arg4); break;
            }
        }
        void ISignalField.ProcessCallback(object callback, object[] args)
        {
            ProcessCallback(callback, args);
        }
        void ISignalField.CancelCallback(object callback) {
            switch (callback)
            {
                case Awaiter awaiter: awaiter.Fail(); break;
            }
        }
        internal SignalProcessorReference _processor;
        SignalProcessor ISignalField.Processor
        {
            get => _processor.Get();
            set => _processor.Set(value);
        }
    }
}