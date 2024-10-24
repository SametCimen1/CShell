#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int sam_cd(char **args);
int sam_help(char **args);
int sam_exit(char **args);
int sam_custom(char **args);
int sam_next(char ** args);

char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "custom",
  "next"
};

int (*builtin_func[]) (char **) = {
  &sam_cd,
  &sam_help,
  &sam_exit,
  &sam_custom,
  &sam_next 
};

int sam_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int sam_next(char **args){

	char defaultCode[100] = "cd /Users/sametcimen/";

	if(args[2] != NULL){
		 strcat(defaultCode, args[1]);
	}else{
		strcpy(defaultCode, "cd ./ ");
	}
	strcat(defaultCode, " && npx create-next-app ");
	if(args[2] != NULL){
		strcat(defaultCode, args[2]);
	}else{
		strcat(defaultCode, args[1]);
	}
	
	printf("%s", defaultCode);
	
	system(defaultCode);

	return 1;
}

int sam_custom(char **args){
	printf("This is a custom function");
	int i;
	for(i = 0; i < 10; i++){
		printf("%d \n", i);
	}	
	return 1;
}


int sam_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "please provide a directory");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int sam_help(char **args)
{
  int i;
  

  printf("Welcome to CShell\n");
  printf("To learn about functions, type its name and hit enter");

  for (i = 0; i < sam_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for more information on other programs.\n");
  return 1;
}

int sam_exit(char **args)
{
  return 0;
}

int sam_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("lsh");
  } else {
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int sam_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    return 1;
  }

  for (i = 0; i < sam_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return sam_launch(args);
}

char *sam_read_line(void)
{
#ifdef SAM_USE_STD_GETLINE
  char *line = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us
  if (getline(&line, &bufsize, stdin) == -1) {
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);  // We received an EOF
    } else  {
      perror("lsh: getline\n");
      exit(EXIT_FAILURE);
    }
  }
  return line;
#else
#define SAM_RL_BUFSIZE 1024
  int bufsize = SAM_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += SAM_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
#endif
}

#define SAM_TOK_BUFSIZE 64
#define SAM_TOK_DELIM " \t\r\n\a"

char **sam_split_line(char *line)
{
  int bufsize = SAM_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, SAM_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += SAM_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, SAM_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

void loopArgs(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = sam_read_line();
    args = sam_split_line(line);
    status = sam_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv)
{
  loopArgs();
  return EXIT_SUCCESS;
}
