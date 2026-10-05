#include<iostream.h>
#include<io.h>
#include<conio.h>
#include<string.h>
#include<stdio.h>
#include<dir.h>
#include<io.h>

//FILEINFO- THIS data is shared all throughout the code
struct FileInfo
{
 char name[64];
 char extension[10];
 char sourcePath[128];
 char destPath[128];
 long sizeBytes;
 int ageDays;
 int isVirtual;
};

//Outcome of ecery task

enum TaskResult
{
  SUCCESS,FAILURE,SKIPPED
};

//Error detection for resilience engine
enum ErrorType
{
 ERR_FOLDER_NOT_FOUND,
 ERR_FILE_NOT_FOUND,
 ERR_UNKNOWN
};

//TASK=not the main but abstract like skeleton for all tasks

class Task
{
protected:
  char taskName[64];
  FileInfo file;
  int maxRetries;
  int retryCount;
public:
  Task(const char* name,FileInfo f,int retries=2)   //const char* so that user cannot change taskname
  {
    strcpy(taskName,name);
    file=f;
    maxRetries=retries;
    retryCount=0;
  }
  virtual TaskResult execute()=0;
  virtual void describe()=0;
  const char* getTaskName()
  {
     return taskName; // return taskdetails;
  }
  FileInfo getfile()
  {
     return file;   //return file details
  }
  virtual ~Task()
  {
  }
};

// Rule -Abstract class
class Rule
{
 protected:
    char destinationFolder[128];
public:
  Rule(const char* destFolder)
  {
    strcpy(destinationFolder,destFolder);
  }
  virtual int matches(FileInfo file)=0; //every file must write their own matches return 1 if file matches this rule or 0 if it doesnt;
  virtual void describe()=0; //every child mus write their own descibe() to explain what condition it checks
  const char* getDestination()
  {
    return destinationFolder; //return destination folder for matched files
  }

  virtual ~Rule() //virtual destructor
  {
  }
};

//Logger-saves exection history of workflows for user record
class Logger
{
private:
 char logFilePath[128];
 int entryCount; //count how mant entries looged so far
public:
  Logger(const char* path="FLOW.LOG")
  {
   strcpy(logFilePath,path);
   entryCount=0;
  }
  void log(const char* message)
  {
    FILE* f=fopen(logFilePath,"a");
    if(f!=NULL)
    {
      entryCount++;
      fprintf(f,"[Entry # %03d] %s\n",entryCount,message);
      fclose(f);
    }
  }
  void showLog()
  {
   char line[256];
   FILE *f=fopen(logFilePath,"r");
   if(f==NULL)
   {
     cout<<"\n No log file found."<<endl;
     return;
   }
   cout<<"\n-----Execution Log-----"<<endl;
   while(fgets(line,sizeof(line),f)!=NULL)
   {
      cout<<line;
   }
   cout<<"-------------"<<endl;
   fclose(f);
  }

  void clearlog()
  {
   FILE *f =fopen(logFilePath,"w");
    if(f!=NULL)
    {
     fclose(f);
     entryCount=0;
     cout<<"Log Cleared"<<endl;
    }
  }
  int getEntryCount()
  {
    return entryCount;
  }
  ~Logger()
  {
  }
};
// Rename Task 
class RenameTask:public Task
{
  private:
  char newname[50];
  public:
  RenameTask(FileInfo f,const char *newFileName):Task("Rename Task",f)
  {
    strcpy(newname,newFileName);
  }

  void describe()
  {
    cout<<"[Rename Task]"<<file.name<<"->"<<newname<<endl;
  }
  TaskResult execute()
  {
    if(file.isVirtual==1)
    {
      strcpy(file.name,newname);
      cout<<"[SIMULATED] Rename to:"<<newname<<endl;
      return SUCCESS;
    }
  
   // Build source & destination full paths for real files
   char srcFull[256],destFull[256];
   strcpy(srcFull,file.sourcePath);
   strcat(srcFull,file.name);

   strcpy(destFull,file.sourcePath);
   strcat(destFull,newname);

   if (rename(srcFull,destFull)==0)
   {
      strcpy(file.name,newname);
      cout<<"\nRenamed Successfully";
      return SUCCESS;
   }
   else
   {
      cout<<"\nRename Failed";
      return FAILURE;
   }
  }
  ~RenameTask()
  {
  }
};
//Create Folder Task
class Createfoldertask:public Task
{ 
  private:
    char Folderpath[128];
  public:
   Createfoldertask(FileInfo f,const char* path):Task("Create Folder Task",f)
   {
      strcpy(Folderpath,path);
   }
   void describe()
   {
     cout<<"[Create Folder Task]"<<Folderpath<<endl;
   }
   TaskResult execute()
   {
      if(file.isVirtual==1)
      {
        cout<<"Create Folder:"<<Folderpath<<endl;
        return SUCCESS;
      }
      if(access(Folderpath,0)==0)
      {
         cout<<"Folder already exists. Skipping."<<endl;
         return SKIPPED;
      }
      if(mkdir(Folderpath)==0)
      {
        cout<<"Folder Created Succesfully"<<endl;
        return SUCCESS;
      }
      else
      {
        cout<<"Failed to create folder"<<endl;
        return FAILURE;
      }
   }
~Createfoldertask()
{
}
};

class MoveTask : public Task
{
  private:
    char targetFolder[128];

  public:
    MoveTask(FileInfo f, const char* destFolder) : Task("Move Task", f)
    {
      strcpy(targetFolder, destFolder);
    }

    void describe()
    {
      cout << "[Move Task] " << file.name << " -> " << targetFolder << endl;
    }

    TaskResult execute()
    {
      
      if (file.isVirtual == 1)
      {
        strcpy(file.sourcePath, targetFolder);
        cout << "[SIMULATED] Move to: " << targetFolder << endl;
        return SUCCESS;
      }
      
      //source & destination full paths 
      char srcFull[256], destFull[256];
      
      strcpy(srcFull, file.sourcePath);
      strcat(srcFull, file.name);

      strcpy(destFull, targetFolder);
      strcat(destFull, file.name);

      // move operation
      if (rename(srcFull, destFull) == 0)
      {
         // Update the file's current location so subsequent tasks know where it is
         strcpy(file.sourcePath, targetFolder);
         cout << "\nMoved Successfully to " << targetFolder << endl;
         return SUCCESS;
      }
      else
      {
         cout << "\nMove Failed (Check if destination folder exists)" << endl;
         return FAILURE;
      }
    }

    ~MoveTask()
    {
    }
};

class WorkFlowManager
{
private:
   char workflowName[64];
   Task* taskList[20]; //array of Task Pointers
   int taskCount;
   Logger logger; //logs records
public:
   WorkFlowManager(const char* name) : logger("FLOW.LOG")
   {
       strcpy(workflowName,name);
       taskCount=0;
   }
   void addTask(Task* task)
   {
    if(taskCount<20)
    {
       taskList[taskCount]=task;
       taskCount++;
       cout<<"task added:"<<task->getTaskName()<<endl;
    }
    else
    {
       cout<<"Worflow Full! Max 20 tasks"<<endl;
    }
   }
   void showTasks()
   {
      cout<<"\nWorkFlow:"<<workflowName;
      cout<<"\n-------------- ";
      if(taskCount==0)
      {
	 cout<<"No tasks added yet"<<endl;
	 return;
      }
      for(int i=0;i<taskCount;i++)
      {
	 cout<<" "<<i+1<<".";
	 taskList[i]->describe();
      }
   }
   void executeWorkFlow()
   {
     cout<<"\nStarting WorkFlow:"<<workflowName<<endl;
     cout<<"------------------"<<endl;
     for(int i=0;i<taskCount;i++)
     {
       cout<<"\nStep"<<i+1<<":";
       taskList[i]->describe();
       logger.log(taskList[i]->getTaskName()); //log that we are starting this task
       TaskResult result=taskList[i]->execute();
       if(result==SUCCESS)
       {
	 cout<<"SUCCESS"<<endl;
	 logger.log("SUCCESS");
       }
       else if(result==FAILURE)
       {
	  cout<<"FAILED"<<endl;
	  logger.log("FAILURE");
	  //call resilience engine
       }
       else if(result==SKIPPED)
       {
	  cout<<"SKIPPED"<<endl;
	  logger.log("SKIPPED");
       }
     }
      cout<<"\n----------------------------"<<endl;
      cout<<"Workflow  complete"<<workflowName<<endl;
      logger.log("Workflow Complete");
    }

    const char* getName()
    {
      return workflowName;
    }
    int getCount()
    {
      return taskCount;
    }
    ~WorkFlowManager()
    {
       for(int i=0;i<taskCount;i++)
       {
	 delete taskList[i];
       }
    }
};

int main()
{
  clrscr();
  int choice;
  Logger sysLogger("FLOW.LOG");
  do
  {
     cout<<"-----------------------------------"<<endl;
     cout<<"             F L O W F O R G E     "<<endl;
     cout<<"  Workflow Automation & Smart Sort "<<endl;
     cout<<"-----------------------------------"<<endl;

     cout<<"\n1.Create New Workflow";
     cout<<"\n2.View Wokflow Tasks";
     cout<<"\n3.Execute WorlFlow";
     cout<<"\n4.View Execution log";
     cout<<"\n5.Clear Log";
     cout<<"\n6. Exit";
     cout<<endl;
     cout<<"\nEnter choice:";
     cin>>choice;
     switch(choice)
     {
	case 1:
	   cout<<"\nCreate Workflow"<<endl;
	   break;
	case 2:
	   cout<<"\nView Tasks"<<endl;
	   break;
	case 3:
	   cout<<"\nExecute WorlFlow"<<endl;
	   break;
	case 4:
	   cout<<"\nView Execution Log"<<endl;
     sysLogger.showLog();
     getch();
	   break;
	case 5:
	    cout<<"\nClear Log"<<endl;
      sysLogger.clearlog();
      getch();
	    break;
	case 6:
	   cout<<"\nGoodbye"<<endl;
	   break;
	default:
	   cout<<"\nINvalid choice";
	}
     }while(choice!=6);
      getch(); 
     return 0;

}

