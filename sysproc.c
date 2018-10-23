#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_getppid(void)
{
  return myproc()->parent->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

int
sys_sleep_sec(void)
{
  int n;
  struct rtcdate dp1, dp2;
  // struct inode *ip;

  if(argint(0, &n) < 0)
    return -1;

  cmostime(&dp1);
  int sec, min, hour, day, month, year;
  sec = n + dp1.second;
  min = dp1.minute;
  hour = dp1.hour;
  day = dp1.day;
  month = dp1.month;
  year = dp1.year;
  // ip = idup(myproc()->cwd);

  if(sec >= 60){
    // second
    min += (sec / 60);
    sec %= 60;
    if(min >= 60){
      // minute
      hour += (min / 60);
      min %= 60;
      if(hour >= 24){
        // hour
        day += (hour / 24);
        hour %= 24;

        // day
        int dday = day - dp1.day;
        while(dday != 0){
          if((year % 4 != 0) || (year % 100 == 0 && year % 400 != 0)){
            //平年
            switch (month) {
              case 1:
              case 3:
              case 5:
              case 7:
              case 8:
              case 10:
              case 12:
                if(day > 31){
                  dday = dday -  (31 - dp1.day);
                  month++;
                  day = day - 31;
                }else{
                  dday = 0;
                }
                break;

              case 4:
              case 6:
              case 9:
              case 11:
                if(day > 30){
                  dday = dday -  (30 - dp1.day);
                  month++;
                  day = day - 30;
                }else{
                  dday = 0;
                }
                break;
              case 2:
                if(day > 28){
                  dday = dday -  (28 - dp1.day);
                  month++;
                  day = day - 28;
                }else{
                  dday = 0;
                }
                break;
              default: // month>12
                month = 1;
                year++;
                break;
            }
          }else{
            //うるう年
            switch (month) {
              case 1:
              case 3:
              case 5:
              case 7:
              case 8:
              case 10:
              case 12:
                if(day > 31){
                  dday = dday -  (31 - dp1.day);
                  month++;
                  day = day - 31;
                }else{
                  dday = 0;
                }
                break;

              case 4:
              case 6:
              case 9:
              case 11:
                if(day > 30){
                  dday = dday -  (30 - dp1.day);
                  month++;
                  day = day - 30;
                }else{
                  dday = 0;
                }
                break;
              case 2:
                if(day > 29){
                  dday = dday -  (29 - dp1.day);
                  month++;
                  day = day - 29;
                }else{
                  dday = 0;
                }
                break;
              default: // month>12
                month = 1;
                year++;
                break;
            }
          }
        }
      }
    }
  }

  acquiresleep(&slplock);
  int counter = 0;
  while(1){
    if(myproc()->killed){
      releasesleep(&slplock);
      return -1;
    }
    counter++;
    if(counter % 40 == 0) // 40回に1回
      cmostime(&dp2);
    if(sec <= dp2.second && min <= dp2.minute && hour <= dp2.hour &&
      day <= dp2.day && month <= dp2.month && year <= dp2.year){
        break;
    }
  }
  releasesleep(&slplock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_getdate(void)
{
  struct rtcdate *dp;
  if (argptr(0, (char **)&dp, sizeof(dp)) < 0)
      return -1;
  cmostime(dp);
  return 0;
}
