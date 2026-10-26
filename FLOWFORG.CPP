#include<iostream.h>
#include<conio.h>
#include<string.h>
#include<stdio.h>
#include<dir.h>
#include<io.h>
#include <sys/stat.h>
#include <time.h>

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
  void setFile(FileInfo f)
  {
     file = f;
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
  void setDestination(const char* folder)
  {
    strcpy(destinationFolder, folder);
  }

  virtual ~Rule() //virtual destructor
  {
  }
};
class FileTypeRule : public Rule
{
    char extension[10];

public:
    FileTypeRule(const char* ext, const char* folder)
        : Rule(folder)
    {
        strcpy(extension, ext);
    }

    int matches(FileInfo file)
    {
      return strcmpi(file.extension, extension) == 0;
    }

    void describe()
    {
        cout << extension << " -> "
             << destinationFolder << endl;
    }
};

class SizeRule : public Rule
{
private:
    long minSizeBytes;

public:
    SizeRule(long size, const char* folder)
        : Rule(folder)
    {
        minSizeBytes = size;
    }

    int matches(FileInfo file)
    {
        if (file.sizeBytes >= minSizeBytes)
            return 1;
        else
            return 0;
    }

    void describe()
    {
        cout << "Files >= " << minSizeBytes
             << " bytes -> "
             << destinationFolder << endl;
    }
};


class AgeRule : public Rule
{
private:
    int minAgeDays;

public:
    AgeRule(int days, const char* folder)
        : Rule(folder)
    {
        minAgeDays = days;
    }

    int matches(FileInfo file)
    {
        if (file.ageDays >= minAgeDays)
            return 1;
        else
            return 0;
    }

    void describe()
    {
        cout << "Files >= " << minAgeDays
             << " days old -> "
             << destinationFolder << endl;
    }
};

int samePathIgnoreCase(const char* first, const char* second)
{
  char firstPath[256];
  char secondPath[256];
  int firstLength;
  int secondLength;
  int i;

  strcpy(firstPath, first);
  strcpy(secondPath, second);
  for(i=0; firstPath[i] != '\0'; i++)
    if(firstPath[i] == '/') firstPath[i] = '\\';
  for(i=0; secondPath[i] != '\0'; i++)
    if(secondPath[i] == '/') secondPath[i] = '\\';

  firstLength = strlen(firstPath);
  secondLength = strlen(secondPath);
  while(firstLength > 1 && firstPath[firstLength - 1] == '\\')
    firstPath[--firstLength] = '\0';
  while(secondLength > 1 && secondPath[secondLength - 1] == '\\')
    secondPath[--secondLength] = '\0';

  return strcmpi(firstPath, secondPath) == 0;
}

int normalizeFolderPath(char* path, int capacity)
{
  int length=strlen(path);
  int i;

  while(length > 0 && (path[length - 1] == '\n' || path[length - 1] == '\r'))
    path[--length]='\0';

  for(i=0;i<length;i++)
    if(path[i] == '/') path[i]='\\';

  if(length == 0)
    return 0;

  if(path[length - 1] != '\\')
  {
    if(length + 1 >= capacity)
      return 0;
    path[length++]='\\';
    path[length]='\0';
  }
  return 1;
}

void buildFilePath(char* result, const char* folder, const char* name,
                   const char* extension)
{
  int length;
  strcpy(result, folder);
  length = strlen(result);
  if(length > 0 && result[length - 1] != '\\' && result[length - 1] != '/')
    strcat(result, "\\");
  strcat(result, name);
  strcat(result, ".");
  strcat(result, extension);
}

int filesHaveSameContents(const char* first, const char* second)
{
  FILE *firstFile;
  FILE *secondFile;
  char firstBuffer[512];
  char secondBuffer[512];
  size_t firstRead;
  size_t secondRead;
  int identical=1;

  firstFile=fopen(first, "rb");
  secondFile=fopen(second, "rb");
  if(firstFile == NULL || secondFile == NULL)
  {
    if(firstFile != NULL) fclose(firstFile);
    if(secondFile != NULL) fclose(secondFile);
    return 0;
  }

  do
  {
    firstRead=fread(firstBuffer, 1, sizeof(firstBuffer), firstFile);
    secondRead=fread(secondBuffer, 1, sizeof(secondBuffer), secondFile);
    if(firstRead != secondRead ||
       memcmp(firstBuffer, secondBuffer, firstRead) != 0)
    {
      identical=0;
      break;
    }
  } while(firstRead > 0);

  if(ferror(firstFile) || ferror(secondFile))
    identical=0;
  fclose(firstFile);
  fclose(secondFile);
  return identical;
}

int copyFileBytes(const char* sourcePath, const char* destinationPath)
{
  FILE *sourceFile;
  FILE *destinationFile;
  char buffer[512];
  size_t bytesRead;
  int success=1;

  sourceFile=fopen(sourcePath, "rb");
  if(sourceFile == NULL)
  {
    cout << "\nCannot open source file: " << sourcePath << endl;
    perror("fopen source");
    return 0;
  }

  destinationFile=fopen(destinationPath, "wb");
  if(destinationFile == NULL)
  {
    cout << "\nCannot create destination file: " << destinationPath << endl;
    perror("fopen destination");
    fclose(sourceFile);
    return 0;
  }

  while((bytesRead=fread(buffer, 1, sizeof(buffer), sourceFile)) > 0)
  {
    if(fwrite(buffer, 1, bytesRead, destinationFile) != bytesRead)
    {
      success=0;
      break;
    }
  }
  if(ferror(sourceFile) || ferror(destinationFile))
    success=0;

  if(fclose(sourceFile) != 0)
    success=0;
  if(fclose(destinationFile) != 0)
    success=0;

  if(!success)
  {
    cout << "\nFile copy failed; the source file was kept." << endl;
    remove(destinationPath);
    return 0;
  }
  return 1;
}

class ResilienceEngine
{
public:
    ErrorType detectError(FileInfo file)
    {
        char path[256];

        // Check whether the source folder exists
        if (access(file.sourcePath, 0) != 0)
            return ERR_FOLDER_NOT_FOUND;

        // Build the complete file path, even if a saved path lacked a slash.
        buildFilePath(path, file.sourcePath, file.name, file.extension);

        // Check whether the file exists
        if (access(path, 0) != 0)
            return ERR_FILE_NOT_FOUND;

        // No missing folder or file detected
        return ERR_UNKNOWN;
    }
    int autoFix(FileInfo f,ErrorType error)
    {
      if(error==ERR_FOLDER_NOT_FOUND)
      {
         if(mkdir(f.sourcePath)==0)
         {
          cout<<"\n[AutoFix] Created Missing Folder:"<<f.sourcePath<<endl;
          return 1;
         }
         else
         {
          cout<<"[AutoFix] Could Not create Folder:"<<f.sourcePath<<endl;
          return 0;
         }
      }
    }
    TaskResult retry(Task *task,int Maxattempts)
    {
      int attempt;
      for(attempt=1;attempt<=Maxattempts;attempt++)
      {
        cout<<"\n[Retry]Attempt "<<attempt<<"of"<<Maxattempts<<endl;
        TaskResult result=task->execute();
        if(result==SUCCESS)
        {
          cout<<"[Retry]Succeded on attempt"<<attempt<<endl;
          return SUCCESS;
        }
        cout<<"[Retry] Attempt"<<attempt<<"failed"<<endl;

      }
      cout << "[Retry] All"<<Maxattempts<<"attempts failed."<<endl;
      return FAILURE;

    }
    TaskResult handle(FileInfo file)
{
    ErrorType error = detectError(file);

    switch (error)
    {
        case ERR_FOLDER_NOT_FOUND:
            cout << "Error: Source folder not found." << endl;
            return FAILURE;

        case ERR_FILE_NOT_FOUND:
            cout << "Error: Source file not found." << endl;
            return FAILURE;

        default:
            cout << "File and folder are present." << endl;
            return SUCCESS;
    }
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
   buildFilePath(srcFull, file.sourcePath, file.name, file.extension);
   buildFilePath(destFull, file.sourcePath, newname, file.extension);

   if(strcmpi(file.name, newname) == 0)
   {
      cout << "\nFile already has the requested name." << endl;
      return SUCCESS;
   }

   if (rename(srcFull,destFull)==0)
   {
      strcpy(file.name,newname);
      cout<<"\nRenamed Successfully";
      return SUCCESS;
   }
   else
   {
      cout<<"\nRename Failed";
      cout<<"\nSource file: "<<srcFull;
      cout<<"\nDestination: "<<destFull;
      perror("\nrename");
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
      char folderToCreate[128];
      int pathLength;
      strcpy(folderToCreate, Folderpath);
      pathLength = strlen(folderToCreate);
      while(pathLength > 1 &&
            (folderToCreate[pathLength - 1] == '\\' ||
             folderToCreate[pathLength - 1] == '/') &&
            !(pathLength == 3 && folderToCreate[1] == ':'))
      {
         folderToCreate[pathLength - 1] = '\0';
         pathLength--;
      }

      if(access(folderToCreate,0)==0)
      {
         cout<<"Folder already exists. Skipping."<<endl;
         return SKIPPED;
      }
      if(mkdir(folderToCreate)==0)
      {
        cout<<"Folder Created Successfully: "<<folderToCreate<<endl;
        return SUCCESS;
      }
      else
      {
        cout<<"Failed to create folder: "<<folderToCreate<<endl;
        perror("mkdir");
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
      
      buildFilePath(srcFull, file.sourcePath, file.name, file.extension);
      buildFilePath(destFull, targetFolder, file.name, file.extension);

      if(samePathIgnoreCase(srcFull, destFull))
      {
         strcpy(file.sourcePath, targetFolder);
         cout << "\nFile is already in the destination folder." << endl;
         return SUCCESS;
      }

      if(access(destFull, 0) == 0)
      {
         cout << "\nMove Failed: destination file already exists: "
              << destFull << endl;
         return FAILURE;
      }

      // Copy then remove the source; this also supports cross-folder moves
      // in DOS runtimes where rename() only handles same-directory renames.
      if(!copyFileBytes(srcFull, destFull))
         return FAILURE;

      if(remove(srcFull) != 0)
      {
         cout << "\nMove Failed: could not remove the source after copying.";
         cout << "\nSource file: " << srcFull;
         cout << "\nDestination file: " << destFull << endl;
         perror("remove source");
         remove(destFull);
         return FAILURE;
      }

      // Update the file's current location so subsequent tasks use it.
      strcpy(file.sourcePath, targetFolder);
      cout << "\nMoved Successfully to " << targetFolder << endl;
      return SUCCESS;
    }

    ~MoveTask()
    {
    }
};
// Backup Task
class BackupTask : public Task
{
private:
    char backupPath[128];

public:

    BackupTask(FileInfo f, const char* destination)
        : Task("Backup Task", f)
    {
        strcpy(backupPath, destination);
    }

    void describe()
    {
        cout << "[Backup Task] "
             << file.name << " -> "
             << backupPath << endl;
    }

    TaskResult execute()
    {
        char sourceFilePath[256];
        char backupFilePath[256];

        // Create complete source file path
        buildFilePath(sourceFilePath, file.sourcePath,
                      file.name, file.extension);

        // Create complete backup file path
        buildFilePath(backupFilePath, backupPath,
                      file.name, file.extension);

        // Open original file for binary reading
        FILE *sourceFile = fopen(sourceFilePath, "rb");

        if (sourceFile == NULL)
        {
            cout << "\nSource file not found: " << sourceFilePath << endl;
            perror("fopen source");
            return FAILURE;
        }

        // Open backup file for binary writing
        FILE *backupFile = fopen(backupFilePath, "wb");

        if (backupFile == NULL)
        {
            cout << "\nCannot create backup file: " << backupFilePath << endl;
            perror("fopen backup");
            fclose(sourceFile);
            return FAILURE;
        }

        // Read and copy one byte at a time
        int byteData;

        while ((byteData = fgetc(sourceFile)) != EOF)
        {
            fputc(byteData, backupFile);
        }

        // Close both files
        fclose(sourceFile);
        fclose(backupFile);

        cout << "\nBackup created successfully." << endl;
        cout << "Backup location: " << backupFilePath << endl;

        return SUCCESS;
    }

    ~BackupTask()
    {
    }
};
// Verify Task
class VerifyTask : public Task
{
public:
    VerifyTask(FileInfo f)
        : Task("Verify Task", f)
    {
    }

    void describe()
    {
        cout << "[Verify Task] Checking file: "
             << file.name << endl;
    }

    TaskResult execute()
    {
        char fullPath[256];

        // Create the complete file path
        buildFilePath(fullPath, file.sourcePath,
                      file.name, file.extension);

        // Open the file for reading
        FILE *fp = fopen(fullPath, "rb");

        if (fp == NULL)
        {
            cout << "\nVerification Failed: File not found."
                 << " Path: " << fullPath << endl;
            perror("fopen");
            return FAILURE;
        }

        // Move to the end of the file
        if (fseek(fp, 0L, SEEK_END) != 0)
        {
            fclose(fp);
            cout << "\nVerification Failed: Cannot read file size."
                 << endl;
            return FAILURE;
        }

        // Get the file size
        long fileSize = ftell(fp);

        fclose(fp);

        if (fileSize < 0)
        {
            cout << "\nVerification Failed: Cannot determine file size."
                 << endl;
            return FAILURE;
        }

        if (fileSize == 0)
        {
            cout << "\nVerification Failed: File is empty."
                 << endl;
            return FAILURE;
        }

        cout << "\nVerification Successful!"
             << "\nFile exists and contains data."
             << "\nFile size: " << fileSize << " bytes"
             << endl;

        return SUCCESS;
    }

    ~VerifyTask()
    {
    }
};

class SmartSortEngine
{
  private:
    Rule* ruleList[10];
    int ruleCount;
  public:
    SmartSortEngine()
    {
       ruleCount=0;
    }
    void addRule(Rule *r)
    {
      if(ruleCount<10)
      {
        ruleList[ruleCount]=r;
        ruleCount++;
      }
      else
      {
        cout<<"\nRule list full! Max 10 Rules allowed"<<endl;
      }
    }
    int getMatchingDestinations(FileInfo file, char destinations[10][128])
    {
      int destinationCount=0;
      int j;
      int folderLength;
      char normalizedFolder[128];
      for(int i=0;i<ruleCount;i++)
      {
        if(ruleList[i]->matches(file)==1)
        {
          strcpy(normalizedFolder, ruleList[i]->getDestination());
          folderLength = strlen(normalizedFolder);
          if(folderLength > 0 && normalizedFolder[folderLength - 1] != '\\' &&
             normalizedFolder[folderLength - 1] != '/' && folderLength < 127)
          {
            strcat(normalizedFolder, "\\");
          }

          for(j=0;j<destinationCount;j++)
          {
            if(samePathIgnoreCase(destinations[j], normalizedFolder))
              break;
          }
          if(j == destinationCount)
          {
            strcpy(destinations[destinationCount], normalizedFolder);
            destinationCount++;
          }
        }
      }
      return destinationCount;
    }
    void showAllRules()
    {
      cout<<"\n-------Smart sort Rules--------"<<endl;
      if(ruleCount==0)
      {
        cout<<"\nNo rules added yet";
        return ;
      }
      for(int i=0;i<ruleCount;i++)
      {
        cout<<i+1<<". ";
        ruleList[i]->describe();
      }
      cout<<"------------------"<<endl;
    }
    int getRuleCount()
    {
      return ruleCount;
    }
    int updateRuleDestination(int ruleNumber, const char* folder)
    {
      if(ruleNumber < 1 || ruleNumber > ruleCount)
        return 0;
      ruleList[ruleNumber - 1]->setDestination(folder);
      return 1;
    }
    ~SmartSortEngine()
    {
      for(int i=0;i<ruleCount;i++)
      {
        delete ruleList[i];
      }
    }
};

class SmartSortTask : public Task
{
  private:
    SmartSortEngine *engine;

  public:
      SmartSortTask(FileInfo f, SmartSortEngine *sortEngine)
                    :Task("SmartSort Task", f)
    {
        engine = sortEngine;
    }
    void describe()
    {
        cout << "[SmartSort Task] " << file.name << "." << file.extension
             << " -> destination selected by saved rules" << endl;
    }

    TaskResult execute();
};
class WorkFlowManager
{
private:
   char workflowName[64];
   Task* taskList[20]; //array of Task Pointers
   int taskCount;
   SmartSortEngine *smartSortEngine;
   Logger logger; //logs records
public:
   WorkFlowManager(const char* name) : logger("FLOW.LOG")
   {
       strcpy(workflowName,name);
       taskCount=0;
       smartSortEngine=NULL;
   }
    SmartSortEngine* getSmartSortEngine()
    {
       if(smartSortEngine == NULL)
          smartSortEngine = new SmartSortEngine();
       return smartSortEngine;
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
     FileInfo currentFile;
     int hasFile=0;
     int i;

     for(i=0;i<taskCount;i++)
     {
       currentFile=taskList[i]->getfile();
       if(currentFile.name[0] != '\0')
       {
         hasFile=1;
         break;
       }
     }

     cout<<"\nStarting WorkFlow:"<<workflowName<<endl;
     cout<<"------------------"<<endl;
     for(i=0;i<taskCount;i++)
     {
       if(hasFile)
         taskList[i]->setFile(currentFile);

       cout<<"\nStep"<<i+1<<":";
       taskList[i]->describe();
       logger.log(taskList[i]->getTaskName()); //log that we are starting this task
        
        TaskResult result=taskList[i]->execute();
       if(result==SUCCESS)
       {
	 if(hasFile)
	   currentFile=taskList[i]->getfile();
	 cout<<"SUCCESS"<<endl;
	 logger.log("SUCCESS");
       }
       else if(result==FAILURE)
       {
	  cout<<"FAILED"<<endl;
	  logger.log("FAILURE");
       }
       else if(result==SKIPPED)
       {
	  cout<<"SKIPPED"<<endl;
	  logger.log("SKIPPED");
       }
      cout<<"\n----------------------------"<<endl;
      }
      if(hasFile)
      {
        for(i=0;i<taskCount;i++)
          taskList[i]->setFile(currentFile);
      }
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
       delete smartSortEngine;
    }
};

int selectWorkflow(WorkFlowManager* workflows[], int workflowCount)
{
   int i, selection;

   if(workflowCount == 0)
   {
      cout << "\nNo workflows have been created yet." << endl;
      return -1;
   }

   cout << "\n--- Workflows ---";
   for(i = 0; i < workflowCount; i++)
      cout << "\n" << i + 1 << ". " << workflows[i]->getName();

   cout << "\nSelect workflow (0 to cancel): ";
   cin >> selection;

   if(selection < 1 || selection > workflowCount)
      return -1;

   return selection - 1;
}

// PathManager - saves and loads user folder paths from PATHS.DAT
// User picks a path by number or adds a new one — no typing needed again
class PathManager
{
private:
    char paths[10][128]; // up to 10 saved paths
    int count;           // how many paths saved so far

public:
    // Constructor - loads saved paths from PATHS.DAT on startup
    PathManager()
    {
        count = 0;
        load();
    }

    // load - reads saved paths from PATHS.DAT into array
    void load()
    {
        int len;
        char buf[128];
        FILE* f = fopen("PATHS.DAT", "r");
        if(f == NULL) return;
        while(count < 10)
        {
            if(fgets(buf, 128, f) == NULL) break;
            len = strlen(buf);
            while(len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
                buf[--len] = '\0';
            if(!normalizeFolderPath(buf, 128))
                continue;
            strcpy(paths[count], buf);
            count++;
        }
        fclose(f);
    }

    // save - writes all paths back to PATHS.DAT
    int save()
    {
        int i;
        FILE* f = fopen("PATHS.DAT", "w");
        if(f == NULL)
        {
            cout << "\nCould not save PATHS.DAT." << endl;
            perror("PATHS.DAT");
            return 0;
        }
        for(i = 0; i < count; i++)
            fprintf(f, "%s\n", paths[i]);
        fclose(f);
        return 1;
    }

    // addPath - adds a new path to the list and saves immediately
    void addPath(char* p)
    {
        char normalizedPath[128];
        int i;

        strcpy(normalizedPath, p);
        if(!normalizeFolderPath(normalizedPath, 128))
        {
            cout << "\nPath is empty or too long to save." << endl;
            return;
        }

        for(i=0;i<count;i++)
        {
            if(samePathIgnoreCase(paths[i], normalizedPath))
            {
                strcpy(p, paths[i]);
                cout << "\nPath is already saved." << endl;
                return;
            }
        }

        if(count < 10)
        {
            strcpy(paths[count], normalizedPath);
            strcpy(p, normalizedPath);
            count++;
            if(save())
                cout << "\nPath saved for future use!" << endl;
            else
                cout << "\nPath can be used now, but was not saved for later." << endl;
        }
        else
        {
            cout << "\nPath list full! Max 10 paths." << endl;
        }
    }

    void deleteSavedPath()
    {
        int selection;
        int i;

        if(count == 0)
        {
            cout << "\nNo saved paths to delete." << endl;
            return;
        }

        cout << "\n--- Saved Paths ---";
        for(i = 0; i < count; i++)
            cout << "\n" << i + 1 << ". " << paths[i];

        cout << "\nSelect path to delete (0 to cancel): ";
        cin >> selection;

        if(selection == 0)
            return;
        if(selection < 1 || selection > count)
        {
            cout << "\nInvalid path selection." << endl;
            return;
        }

        for(i = selection - 1; i < count - 1; i++)
            strcpy(paths[i], paths[i + 1]);
        count--;
        if(save())
            cout << "\nSaved path removed. The actual folder was not deleted." << endl;
        else
            cout << "\nPath removed for this session, but PATHS.DAT could not be updated."
                 << endl;
    }

    // selectPath - shows saved paths, user picks one or adds new

void selectPath(char* result)
{
    int i, ch, parentChoice, baseLength, childLength;
    char subPath[128];

    cout << "\n--- Select Folder ---";

    for(i = 0; i < count; i++)
        cout << "\n" << i + 1 << ". " << paths[i];

    cout << "\n" << count + 1 << ". Extend a saved path";
    cout << "\n" << count + 2 << ". Enter a completely new path";
    cout << "\nEnter choice: ";
    cin >> ch;

    if(ch >= 1 && ch <= count)
    {
        strcpy(result, paths[ch - 1]);
        if(!normalizeFolderPath(result, 128))
        {
            cout << "\nSaved folder path is invalid." << endl;
            strcpy(result, ".\\");
        }
    }
    else if(ch == count + 1)
    {
        if(count == 0)
        {
            cout << "\nThere are no saved paths to extend." << endl;
            strcpy(result, ".\\");
            return;
        }

        cout << "\nSelect saved path to extend:";
        for(i = 0; i < count; i++)
            cout << "\n" << i + 1 << ". " << paths[i];
        cout << "\nEnter choice: ";
        cin >> parentChoice;

        if(parentChoice < 1 || parentChoice > count)
        {
            cout << "\nInvalid path choice. Using current directory." << endl;
            strcpy(result, ".\\");
            return;
        }

        cout << "\nEnter subfolder path to add (example: CourseProject): ";
        cin >> subPath;

        strcpy(result, paths[parentChoice - 1]);
        if(!normalizeFolderPath(result, 128))
        {
            cout << "\nSaved folder path is invalid." << endl;
            strcpy(result, ".\\");
            return;
        }
        baseLength = strlen(result);
        if(baseLength > 0 && result[baseLength - 1] == '\\')
            result[--baseLength] = '\0';
        childLength = strlen(subPath);
        if(baseLength + childLength + 3 > 128)
        {
            cout << "\nCombined path is too long." << endl;
            strcpy(result, paths[parentChoice - 1]);
            return;
        }

        strcat(result, "\\");
        strcat(result, subPath);
        if(!normalizeFolderPath(result, 128))
        {
            cout << "\nCombined path is too long." << endl;
            strcpy(result, paths[parentChoice - 1]);
            return;
        }

        addPath(result);
    }
    else if(ch == count + 2)
    {
        cout << "\nEnter complete folder path: ";
        cin >> result;

        if(!normalizeFolderPath(result, 128))
        {
            cout << "\nPath is empty or too long." << endl;
            strcpy(result, ".\\");
            return;
        }

        addPath(result);
    }
    else
    {
        cout << "\nInvalid choice. Using current directory.\n";
        strcpy(result, ".\\");
    }
}

};

TaskResult SmartSortTask::execute()
{
    char fullPath[256];
    char destinations[10][128];
    char destinationFiles[10][256];
    struct stat fileStat;
    time_t currentTime;
    double seconds;
    int destinationCount;
    int i;
    int sourceIsDestination=0;
    int copyFailed=0;
    FILE *input;
    FILE *output;
    char buffer[512];
    size_t bytesRead;

    if(engine == NULL)
        return FAILURE;

    buildFilePath(fullPath, file.sourcePath, file.name, file.extension);

    if(stat(fullPath, &fileStat) != 0)
    {
        cout << "\nError: Cannot access file: " << fullPath << endl;
        return FAILURE;
    }

    file.sizeBytes = fileStat.st_size;
    currentTime = time(NULL);
    seconds = difftime(currentTime, fileStat.st_mtime);
    file.ageDays = (int)(seconds / 86400);

    cout << "\nActual file size: " << file.sizeBytes << " bytes";
    cout << "\nFile age: " << file.ageDays << " days" << endl;

    destinationCount = engine->getMatchingDestinations(file, destinations);
    if(destinationCount == 0)
    {
        cout << "\nNo SmartSort rule matched this file." << endl;
        return SKIPPED;
    }

    // A file may match several rules. Copy it to each unique destination,
    // then remove the source only after all destination copies succeed.
    for(i=0;i<destinationCount;i++)
    {
        buildFilePath(destinationFiles[i], destinations[i],
                      file.name, file.extension);

        if(samePathIgnoreCase(fullPath, destinationFiles[i]))
        {
            sourceIsDestination=1;
            continue;
        }

        if(access(destinationFiles[i], 0) == 0)
        {
            if(filesHaveSameContents(fullPath, destinationFiles[i]))
                continue;
            cout << "\n[SmartSort] Destination file already exists with different contents: "
                 << destinationFiles[i] << endl;
            copyFailed=1;
            break;
        }

        input=fopen(fullPath, "rb");
        if(input == NULL)
        {
            cout << "\n[SmartSort] Could not open source file: " << fullPath << endl;
            copyFailed=1;
            break;
        }

        output=fopen(destinationFiles[i], "wb");
        if(output == NULL)
        {
            fclose(input);
            cout << "\n[SmartSort] Could not create destination file: "
                 << destinationFiles[i] << endl;
            perror("fopen");
            copyFailed=1;
            break;
        }

        while((bytesRead=fread(buffer, 1, sizeof(buffer), input)) > 0)
        {
            if(fwrite(buffer, 1, bytesRead, output) != bytesRead)
            {
                copyFailed=1;
                break;
            }
        }
        if(ferror(input))
            copyFailed=1;
        if(ferror(output))
            copyFailed=1;

        fclose(input);
        fclose(output);

        if(copyFailed)
        {
            remove(destinationFiles[i]);
            cout << "\n[SmartSort] Failed while copying to "
                 << destinations[i] << endl;
            break;
        }
    }

    if(copyFailed)
    {
        // Remove copies created during this attempt so the source remains intact.
        while(i > 0)
        {
            i--;
            if(!samePathIgnoreCase(fullPath, destinationFiles[i]) &&
               access(destinationFiles[i], 0) == 0)
                remove(destinationFiles[i]);
        }
        return FAILURE;
    }

    if(!sourceIsDestination && remove(fullPath) != 0)
    {
        cout << "\n[SmartSort] Copies were made, but the source could not be removed: "
             << fullPath << endl;
        perror("remove");
        return FAILURE;
    }

    for(i=0;i<destinationCount;i++)
        cout << "[SmartSort] Routed " << file.name << "." << file.extension
             << " to folder " << destinations[i] << endl;

    if(destinationCount > 0)
    {
        strcpy(file.destPath, destinations[0]);
        strcpy(file.sourcePath, destinations[0]);
    }
    return SUCCESS;
}

int main()
{
  clrscr();
  int choice;

  WorkFlowManager *workflows[10];
  int workflowCount = 0;
  WorkFlowManager *workflow = NULL;
  Logger sysLogger("FLOW.LOG");
  PathManager pm; // loads saved paths from PATHS.DAT automatically
  do
   {
      cout<<"-----------------------------------"<<endl;
      cout<<"             F L O W F O R G E     "<<endl;
      cout<<"  Workflow Automation & Smart Sort "<<endl;
      cout<<"-----------------------------------"<<endl;

     cout<<"\n1.Create New Workflow";
     cout<<"\n2.View Workflow Tasks";
     cout<<"\n3.Execute Workflow";
     cout<<"\n4.View Execution Log";
     cout<<"\n5.Clear Log";
     cout<<"\n6.Delete Workflow";
     cout<<"\n7.Change SmartSort Rule Destination";
     cout<<"\n8.Delete Saved Folder Path";
     cout<<"\n9.Exit";
     cout<<endl;
     cout<<"\nEnter choice:";
     cin>>choice;
      switch(choice)
      {
         case 1:
         {
            char name[64];

            if(workflowCount >= 10)
            {
               cout<<"\nWorkflow list is full! Maximum 10 workflows.";
               getch();
               break;
            }

            cout<<"\nEnter Workflow Name:";
            cin>>name;

            workflow=new WorkFlowManager(name);
            workflows[workflowCount] = workflow;
            workflowCount++;

            int taskChoice;
            FileInfo workflowFile;
            int workflowFileSet=0;
            workflowFile.name[0]='\0';
            workflowFile.extension[0]='\0';
            workflowFile.sourcePath[0]='\0';
            workflowFile.destPath[0]='\0';
            workflowFile.sizeBytes=0;
            workflowFile.ageDays=0;
            workflowFile.isVirtual=0;

            do
            {
               cout<<"\n------ Add Tasks ------";
               cout<<"\n1.Rename File";
               cout<<"\n2.Create Folder";
               cout<<"\n3.Move File";
               cout<<"\n4.Backup File";
               cout<<"\n5.Verify File";
               cout<<"\n6.Add SmartSort Rules and Destinations";
               cout<<"\n7.Add File to SmartSort";
               cout<<"\n0.Finish Adding Tasks";
               cout<<"\nEnter choice:";
               cin>>taskChoice;

               if(taskChoice==0)
               {
                  break;
               }

               if(taskChoice < 1 || taskChoice > 7)
               {
                  cout<<"\nInvalid Task Choice!";
                  continue;
               }

               FileInfo f;
               if(workflowFileSet)
               {
                  f=workflowFile;
               }
               else
               {
                  f.name[0]='\0';
                  f.extension[0]='\0';
                  f.sourcePath[0]='\0';
                  f.destPath[0]='\0';
                  f.sizeBytes=0;
                  f.ageDays=0;
                  f.isVirtual=0;
               }

               if(taskChoice == 7 &&
                  workflow->getSmartSortEngine()->getRuleCount() == 0)
               {
                  cout << "\nConfigure SmartSort rules first using option 6." << endl;
                  continue;
               }

               // Folder creation and SmartSort rule configuration do not need a file.
               if(taskChoice != 2 && taskChoice != 6)
               {
                  if(!workflowFileSet)
                  {
                     cout<<"\nEnter File Name:";
                     cin>>f.name;

                     cout<<"Enter Extension:";
                     cin>>f.extension;

                     cout<<"\nSelect Source Folder:";
                     pm.selectPath(f.sourcePath);

                     workflowFile=f;
                     workflowFileSet=1;
                  }
                  else
                  {
                     f=workflowFile;
                     cout<<"\nUsing workflow file: "<<f.name<<"."
                         <<f.extension<<" from "<<f.sourcePath<<endl;
                  }
               }

               if(taskChoice==1)
               {
                  char newName[50];

                  cout<<"Enter New File Name:";
                  cin>>newName;

                  workflow->addTask(
                     new RenameTask(f,newName));
               }
               else if(taskChoice==2)
               {
                  char parentFolder[128];
                  char folderPath[128];
                  char folderName[64];
                  int parentLength;

                  cout<<"\nSelect Parent Folder:";
                  pm.selectPath(parentFolder);

                  cout<<"Enter New Folder Name:";
                  cin>>folderName;

                  strcpy(folderPath, parentFolder);
                  parentLength = strlen(folderPath);
                  if(parentLength > 0 && folderPath[parentLength - 1] != '\\' &&
                     folderPath[parentLength - 1] != '/')
                  {
                     if(parentLength + strlen(folderName) + 2 > 128)
                     {
                        cout<<"\nCombined folder path is too long."<<endl;
                        continue;
                     }
                     strcat(folderPath, "\\");
                  }
                  else if(parentLength + strlen(folderName) + 1 > 128)
                  {
                     cout<<"\nCombined folder path is too long."<<endl;
                     continue;
                  }
                  strcat(folderPath, folderName);

                  workflow->addTask(
                     new Createfoldertask(f,folderPath));
               }
               else if(taskChoice==3)
               {
                  char destFolder[128];

                  cout<<"\nSelect Destination Folder:";
                  pm.selectPath(destFolder);

                  workflow->addTask(
                     new MoveTask(f,destFolder));
               }
               else if(taskChoice==4)
               {
                  char backupPath[128];

                  cout<<"\nSelect Backup Folder:";
                  pm.selectPath(backupPath);

                  workflow->addTask(
                     new BackupTask(f,backupPath));
               }
               else if(taskChoice == 5)
                {
                    workflow->addTask(new VerifyTask(f));
                }
               
              else if(taskChoice == 6)
                {
                         SmartSortEngine *taskSortEngine = workflow->getSmartSortEngine();

                        int numberOfRules;
                        int availableRules;
                        int ruleType;
                        int i;
                        long minSize;
                        int minAge;
                        char extension[10];
                        char destination[128];

                        availableRules = 10 - taskSortEngine->getRuleCount();
                        if(availableRules <= 0)
                        {
                            cout << "\nSmartSort already has the maximum of 10 rules." << endl;
                            taskSortEngine->showAllRules();
                        }
                        else
                        {
                        cout << "\nCurrently configured rules:" << endl;
                        taskSortEngine->showAllRules();
                        cout << "How many additional SmartSort rules (1-"
                             << availableRules << ")? ";
                        cin >> numberOfRules;

                        if(numberOfRules < 1 || numberOfRules > availableRules)
                            {
                              cout << "\nInvalid number of additional rules!";
                            continue;
                            }

    for(i = 0; i < numberOfRules; i++)
      {
            cout << "\n1. File Type Rule";
            cout << "\n2. File Size Rule";
            cout << "\n3. File Age Rule";
            cout << "\nEnter rule type: ";
            cin >> ruleType;

            if(ruleType == 1)
            {
                cout << "Enter extension without dot: ";
                cin >> extension;

                cout << "Select destination folder for ." << extension << " files:";
                pm.selectPath(destination);

                taskSortEngine->addRule(
                    new FileTypeRule(extension, destination));
            }
            else if(ruleType == 2)
            {
                cout << "Enter minimum size in bytes: ";
                cin >> minSize;

                cout << "Select destination folder for files >= "
                     << minSize << " bytes:";
                pm.selectPath(destination);

                taskSortEngine->addRule(
                    new SizeRule(minSize, destination));
            }
            else if(ruleType == 3)
            {
                cout << "Enter minimum age in days: ";
                cin >> minAge;

                cout << "Select destination folder for files >= "
                     << minAge << " days old:";
                pm.selectPath(destination);

                taskSortEngine->addRule(
                    new AgeRule(minAge, destination));
            }
            else
            {
                cout << "Invalid rule type!";
                i--;
            }
        }
                        }
                }
              else if(taskChoice == 7)
                {
                    SmartSortEngine *taskSortEngine = workflow->getSmartSortEngine();
                    workflow->addTask(
                        new SmartSortTask(f, taskSortEngine));
}


            }while(taskChoice!=0);

            cout<<"\nWorkflow Created Successfully!"<<endl;
            getch();
            break;
         }

         case 2:
         {
            int selected = selectWorkflow(workflows, workflowCount);
            if(selected >= 0)
            {
               workflows[selected]->showTasks();
            }
            getch();
            break;
         }

         case 3:
         {
            int selected = selectWorkflow(workflows, workflowCount);
            if(selected >= 0)
            {
               workflows[selected]->executeWorkFlow();
            }
            getch();
            break;
         }

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
         {
            int selected = selectWorkflow(workflows, workflowCount);
            if(selected >= 0)
            {
               delete workflows[selected];
               for(int i = selected; i < workflowCount - 1; i++)
                  workflows[i] = workflows[i + 1];
               workflowCount--;
               workflow = NULL;
               cout << "\nWorkflow deleted." << endl;
            }
            getch();
            break;
         }

         case 7:
         {
            int selected = selectWorkflow(workflows, workflowCount);
            if(selected >= 0)
            {
               SmartSortEngine *engine = workflows[selected]->getSmartSortEngine();
               if(engine->getRuleCount() == 0)
               {
                  cout << "\nThis workflow has no SmartSort rules yet." << endl;
               }
               else
               {
                  int ruleNumber;
                  char newDestination[128];
                  engine->showAllRules();
                  cout << "Enter rule number to change (0 to cancel): ";
                  cin >> ruleNumber;
                  if(ruleNumber > 0 && ruleNumber <= engine->getRuleCount())
                  {
                     cout << "Select the new destination folder:";
                     pm.selectPath(newDestination);
                     engine->updateRuleDestination(ruleNumber, newDestination);
                     cout << "\nSmartSort destination updated." << endl;
                  }
               }
            }
            getch();
            break;
         }

         case 8:
            pm.deleteSavedPath();
            getch();
            break;

         case 9:
            cout<<"\nGoodbye"<<endl;
            break;

         default:
            cout<<"\nInvalid Choice!"<<endl;
            getch();
      }

   }while(choice!=9);

    for(int i = 0; i < workflowCount; i++)
       delete workflows[i];
    workflow = NULL;

   getch();
   return 0;
}
