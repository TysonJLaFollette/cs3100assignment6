#include<string>
#include<iostream>
#include<sstream>
#include<unistd.h>
#include<vector>
#include<algorithm>
#include<sys/wait.h>
#include <chrono>
#include <fstream>
#include<fcntl.h>
#include<sys/stat.h>
struct CommandSet{
    std::vector<std::string> commands;
    std::string inFile = "";
    std::string outFile = "";
};

void printCommandSet(CommandSet mySet){
    std::cout << "inFile: " << mySet.inFile << "\n";
    std::cout << "outFile: " << mySet.outFile << "\n";
    for (unsigned int i = 0; i < mySet.commands.size(); i++){
        std::cout << "Commands(" << i << "): " << mySet.commands.at(i) << "\n";
    }
}

std::string captureCommand(){
	std::string myCommand;
	std::getline(std::cin,myCommand);
	//std::cout << "Received command: " << myCommand << "\n";
	return myCommand;
}

CommandSet createCommandSet(std::string input){//crams a string into a stuct that understands pipes.
    std::stringstream ss;
    CommandSet result;
    ss << input;
    std::string word;
    std::string currentCommand = "";
    std::vector<std::string> words;
    while(!ss.eof()){
        ss >> word;
        words.push_back(word);
    }
    for(unsigned int i = 0; i < words.size(); i++){
        if (words.at(i) == ">"){
            result.outFile = words.at(i+1);
            std::cout << "Erasing " << words.at(i) << " and " << words.at(i+1) << "\n";
            words.erase(words.begin() + i,words.begin() + i+2);//remove the items now that we're done with them.
        }
    }

    for(unsigned int i = 0; i < words.size(); i++){
        if (words.at(i) == "<"){
            result.inFile = words.at(i+1);
            std::cout << "Erasing " << words.at(i) << " and " << words.at(i+1) << "\n";
            words.erase(words.begin() + i,words.begin() +i+2);//remove the items now that we're done with them.
        }
    }
    //at this point we only have commands seperated by pipes.
    for(unsigned int i = 0; i < words.size(); i++){
        if (words.at(i) == "|"){
            //what do we do if we encounter a pipe?
            //Split into seperate commands at that point.
            result.commands.push_back(currentCommand);
            currentCommand = "";
        }

        else{//grab all words until another special character 
            currentCommand += (" " + words.at(i));
        }
    }
    result.commands.push_back(currentCommand);

    return result;
}

std::vector<char*> parseCommand(std::string commandToParse){//turns a string into an array of seperated words as character pointers.
	std::vector<std::string> words;
	std::stringstream ss;
	std::string word;
	ss << commandToParse;
	while(!ss.eof()){
		ss >> word;
		words.push_back(word);
	}
	std::vector<char*> args;
	for (unsigned int i = 0; i < words.size(); i++){
		args.push_back(const_cast<char*>(words.at(i).data()));
	}
	args.push_back(nullptr);
	//the rest is debugging text.
	//words now contains seperated words as strings.
	/*
	for (int i = 0; i < words.size(); i++){
		std::cout << "Words(" << i << "): " << words.at(i) << "\n";
	}
	//goes through words, 
	
	for (int i = 0; i < words.size();i++){
		std::cout << "Args(" << i << "):" << args.at(i) << "\n";
	}
	for (int i = 0; i < words.size();i++){
		std::cout << "Args.data() at " << i << ": " << args.data()[i] << "\n";
	}
	*/
	return args;
}

int main(){
	bool keepRunning = true;
	bool executionTiming = false;
	std::vector<std::string> history;
	while (keepRunning){
		std::cout << "('ptime' toggles timing. 'exit' to quit.) Prompt: ";
		std::string myCommand = captureCommand();
		history.push_back(myCommand);
        CommandSet myCommandSet = createCommandSet(myCommand);
        printCommandSet(myCommandSet);
        int p[2][2];//a pair of pipes.(uninitialized)
        int cur_in = 0;//current pipe feeding in.
        int cur_out = 1;//current pipe currently feeding out.
        pipe(p[cur_out]);//creates a new pipe into cur_out.
        pipe(p[cur_in]);
        for(unsigned int i = 0; i < myCommandSet.commands.size();i++){//loop through every command in the set
            
            std::vector<char*> args = parseCommand(myCommandSet.commands.at(i));
		    /*for(int i = 0; i < 2; i++){
			    std::cout << "Args at " << i << ": " << args[i] << "\n";
		    }*/
		    if(((std::string)args[0]).compare("^") == 0)
            {
			    //run the selected command from history.
			    std::cout << "Running from history...\n";
			    int selectedIndex = std::stoi((std::string)args[1]);
			    std::vector<char*> historicArgs = parseCommand(history.at(selectedIndex));
			    args.clear();
			    for (unsigned int i = 0; i < historicArgs.size(); i++)
			    {
				    args.push_back(historicArgs[i]);
			    }
			    /*for(int i = 0; i < args.size(); i++){
				    std::cout << "Args at " << i << ": " << args[i] << "\n";
			    }*/
		    }
		    if (((std::string)args[0]).compare("exit") ==0) //cannot simply compare 'exit' to args[0] with == operator, apparently. Cast to string for compare function.
            {
			    keepRunning = false;
		    } 
		    else if (((std::string)args[0]).compare("cd") ==0){
			    //implementation of cd command.
		    }
		    else if (((std::string)args[0]).compare("ptime") ==0)
		    {
			    executionTiming = !executionTiming;
			    if (executionTiming == true)
			    {
				    std::cout << "Exectution timing: ON\n";
			    }
			    else 
			    {
				    std::cout << "Execution timing: OFF\n";
			    }
		    }
		    else if (((std::string)args[0]).compare("history") ==0)
		    {
			    for(unsigned int i = 0; i < history.size(); i++)
			    {
				    std::cout << "(" << i << ")" << history.at(i) << "\n";
			    }
		    }
		    else
		    {   
			    //start timer
			    auto starttime = std::chrono::steady_clock::now();
			    std::cout << "Running: " << myCommandSet.commands.at(i) << "\n";
                pid_t pid = fork();
                if (pid < 0){perror("Error");}
			    if (pid == 0){//if we're the child process.
                int fin = -1;
                int fout = -1;
                if (myCommandSet.commands.size() ==1){
                    if(myCommandSet.inFile != ""){//first command gets input from input file
                        std::cout << "Has infile!\n";
                        fin = open(myCommandSet.inFile.c_str(),O_RDONLY);
                        dup2(fin,STDIN_FILENO);
                    }
                    else{
                        dup2(STDIN_FILENO,STDIN_FILENO);
                    } 
                    if(myCommandSet.outFile != ""){//last command outputs to output file
                        std::cout << "Has outfile!\n";
                        fout = open(myCommandSet.outFile.c_str(),O_WRONLY | O_CREAT);
                        dup2(fout,STDOUT_FILENO);
                    }
                    else{
                        dup2(STDOUT_FILENO,STDOUT_FILENO);
                    }
                }
                else {//if we have more than one command.
                    if (i == 0){
                        if(myCommandSet.inFile != ""){//first command gets input from input file
                        std::cout << "Has infile!\n";
                        fin = open(myCommandSet.inFile.c_str(),O_RDONLY);
                        dup2(fin,STDIN_FILENO);
                        }
                        else{
                            dup2(STDIN_FILENO,STDIN_FILENO);
                        }
                        dup2(p[cur_out][1],STDOUT_FILENO);
                    }
                    else if (i == myCommandSet.commands.size()-1){ 
                        if(myCommandSet.outFile != ""){//last command outputs to output file
                        std::cout << "Has outfile!\n";
                        fout = open(myCommandSet.outFile.c_str(),O_WRONLY | O_CREAT);
                        dup2(fout,STDOUT_FILENO);
                        }
                        else{
                            dup2(STDOUT_FILENO,STDOUT_FILENO);
                        }
                        dup2(p[cur_in][0],STDIN_FILENO);
                    }
                    else{
                            dup2(p[cur_in][0], STDIN_FILENO);
                            dup2(p[cur_out][1],STDOUT_FILENO);
                    }
                }
                   
				    execvp(args[0],args.data());
				    //return an error if execvp actually reaches this point. It shouldn't.
				    perror("Error");
				    exit (EXIT_FAILURE);
			    }    
			    else 
			    {//if we're the parent.
				    int* status = 0;
				    waitpid(pid,status,0);
                    close(p[cur_out][1]);
                    std::swap(cur_in,cur_out);//swaps the input pipe and output pipe for the parent.
				    //stop timer.
				    auto endtime = std::chrono::steady_clock::now();
				    auto timetaken = endtime - starttime;
				    if(executionTiming){std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::microseconds>(timetaken).count() << " microseconds.\n";}
			    }
		    }
        }

		close(p[cur_in][0]);
        close(p[cur_in][1]);
        close(p[cur_out][0]);
        close(p[cur_out][1]);
	}
}

