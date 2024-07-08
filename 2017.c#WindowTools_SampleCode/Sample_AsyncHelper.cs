using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BLContentBuilder
{
    public static class AsyncHelper
    {
        private class Scheduler
        {
            private class ScheduledTask
            {
                internal object _objLock = new object();
                internal readonly Action _action;
                internal System.Timers.Timer Timer;
                internal EventHandler TaskComplete;
                internal string _ID = string.Empty;

                public string ID { get { return _ID; } } 

                public ScheduledTask(string id, Action action, int timeoutMs)
                {
                    _ID = id;
                    _action = action;
                    Timer = new System.Timers.Timer() { Interval = timeoutMs };
                    Timer.Elapsed += TimerElapsed;
                }

                private void TimerElapsed(object sender, System.Timers.ElapsedEventArgs e)
                {
                    lock (_objLock)
                    {
                        Timer.Stop();
                        Timer.Elapsed -= TimerElapsed;
                        Timer = null;

                        _action();
                        TaskComplete(this, EventArgs.Empty);
                    }
                }
            }

            private readonly ConcurrentDictionary<Action, ScheduledTask> _scheduledTasks = new ConcurrentDictionary<Action, ScheduledTask>();

            public bool Execute(string id, Action action, int timeoutMs)
            {
                if (false == id.IsNullOrEmpty() && false == _scheduledTasks.IsNullOrEmpty())
                {
                    bool isAlreadyRoutine = _scheduledTasks.Values.Any(s => { return null != s && id.IsSame(s.ID); });
                    if (true == isAlreadyRoutine)
                        return false;
                }

                if (0 >= timeoutMs)
                {
                    action();
                    return true;
                }
                
                var task = new ScheduledTask(id, action, timeoutMs);
                task.TaskComplete += RemoveTask;
                _scheduledTasks.TryAdd(action, task);
                task.Timer.Start();
                return true;
            }

            private void RemoveTask(object sender, EventArgs e)
            {
                var task = (ScheduledTask)sender;
                task.TaskComplete -= RemoveTask;
                ScheduledTask deleted;
                _scheduledTasks.TryRemove(task._action, out deleted);
            }
        }

        private static Scheduler _asyncScheduler = new Scheduler();

        public static void CallAsyncNotOverlap(string id, Action action, float timeDelaySecond)
        {
            _asyncScheduler.Execute(id, action, (int)(timeDelaySecond * 1000.0f));
        }

        public static void CallAsync(Action action, float timeDelaySecond)
        {
            _asyncScheduler.Execute(string.Empty, action, (int)(timeDelaySecond * 1000.0f));
        }
    }
}
