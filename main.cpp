#include<string>
#include<iostream>
#include<sstream>
#include<unistd.h>
#include<vector>
#include<algorithm>
#include<sys/wait.h>
#include <chrono>

std::string captureCommand(){
	std::string myCommand;
	std::getline(std::cin,myCommand);
	//std::cout << "Received command: " << myCommand << "\n";
	return myCommand;
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
		std::vector<char*> args = parseCommand(myCommand);
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
		else if (((std::string)args[0]).compare("cd") ==0)
		{
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
			pid_t pid = fork();
			if (pid == 0)
			{//if we're the child process.
				execvp(args[0],args.data());
				//return an error if execvp actually reaches this point. It shouldn't.
				perror("Error");
				exit (EXIT_FAILURE);
			} 
			else 
			{//if we're the parent.
				int* status = 0;
				waitpid(pid,status,0);
				//stop timer.
				auto endtime = std::chrono::steady_clock::now();
				auto timetaken = endtime - starttime;
				if(executionTiming){std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::microseconds>(timetaken).count() << "microseconds.\n";}
			}
		}
	}
}

