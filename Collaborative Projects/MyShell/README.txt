Authors: Tanay Kale(https://github.com/tvk134) & Anthony Paulino(https://github.com/antoutsider)

Extensions: 3.2 Home directory & 3.3  Wildcard Directory


Test Plan
---------
if DEBUG macro set to 1 is for testing purposes, make sure DEBUG is set to 0 for all source codes
We have .txt files in a "tests" folder that we will refer to as the testcases

Testing reading/parsing of shell:
---------------------------------
> leading white spaces
> trailing white spaces
> multiple spaces between tokens
> no spaces betweens words and "|,<,>"
    -> Turn on DEBUG macro to 1, prints out the command struct, with its argument list, input and output files [path to command will be included in the argument list, the real argument list for execv starts at index 1]
    -> test case: 'tests/test0.1.txt' (will print errors of file not found for the fake implemented files, purposes was to check reading/parsing)

Testing functionality of shell:
-------------------------------
Commands without piping:
 > Commands with no redirection
    -> using commands that do not require redirections
    -> test case: 'tests/test1.1.txt'

 > Commands with redirection 
    * Single redirection
        -> Using commands that read from a file or write to a file
        -> test case: 'tests/test1.2.txt'

    * multiple redirections
        - 1 input redirection & 1 output redirection
            -> using commands that reads from a file and writes to a file
            -> test case: 'tests/test1.3.txt'
            
        - 2 output/input redirections
            -> using commands that reads from a file and writes to a file twice
            -> test case: 'tests/test1.4.txt'

Commands with piping:
 > Subcommands with no redirection
    -> piping two commands with no redirection
    -> test case: 'tests/test2.1.txt'
    
 > Subcommands with redirection
    * 1 redirection on left-side of pipe or right-side of pipe, or on both side of the pipe
        -> using commands with one redirection on either side or on both sides
        -> test case: 'tests/test2.2.txt'

Commands with the following features:
 > Built-in commands
    * pwd is placed and execute inside of a child process along with the other non built-in commands, since our exec function handles dup2 and changing STDOUT already, if pwd command it will just run the pwd function and write t STDOUT and then exit child process
    * cd with pipe, cd executes the other proccess and then changes directory
    * exit with pipe, exit executes the other proccess and then terminates
    * cd and exit execute and is not affected or takes redirection into consideration
 > Path names
* can run any pathname as an executable as long as it can be executed and found
 > Bare names
    * checks the following directories in write-up, if can't find barename in the following directories, throws an error
 > Wildcards
    * wildcards are not permitted with redirection, it will run valid commands if wildcard only matches 1 file, if multiple it will still try to execute (behavior is not determined)
 > Extensions:
    * HOME
    * Directory Wildcards
 -> using commands with variations of the following features, with redirections and pipe (compound testing)
 -> test case: 'tests/test3.1.txt'


Error detection:
----------------
> Missing paremeters on a command
    * missing executable program
    * directory, file or executable program cannot be found or accessed
    * missing file or executable proram when using redirection
    * multiple redirects to input, ouput, or both (with & without pipe)
    * Wildcards are not permmited in redirection targets
    * pwd # of arguments (takes no arguments)
    * cd # of arguments (takes no or 1 arguments)
    * allows only 1 pipe
      -> test case: 'tests/test4.1.txt'