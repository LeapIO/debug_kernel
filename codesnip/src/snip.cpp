/**
 * @file snip.cpp
 * @brief 
 * @author double_D (18374822143@163.com)
 * @version 1.0
 * @date 2021-02-28
 * @copyright Copyright double_D
 * 
 */
#include "snip.h"
#include "unit.h"

using namespace snip_utils;

int 
AdjustArgs(int i, char* argv[], int argc, unsigned del){
	/* i 代表删第几个 i <= argc-1*/
	if(i>=0){
		for(int j=i+del; j<argc; j++, i++)
			argv[i] = argv[j];
		argv[i] = NULL;
		return argc - del;
	}
	return argc;
}

int
ProcessOptArgs(int argc, char* argv[]){
	Snip& snip = Snip::GetInstance();
restart:
	for(int i=0; i<argc; i++) {
		std::string para = argv[i];
		if(para.find("=") == std::string::npos){continue;}	
		if(para.find("=")>=para.size()-1){continue;}
		const std::string _para(para.substr(0, para.find("=")+1));		
		if(snip.para_num.count(_para)==0){continue;}
		const std::string _len(para.substr(para.find("=")+1));
		if(!snip_utils::StringIsDigit(_len)){continue;}

		switch(snip.para_num[_para]){
		case SERVSE_T:	
			snip.server_type = std::stoi(_len);
			argc = AdjustArgs(i, argv, argc, 1);
			goto restart;
		case BLOCK_IO_T:
			snip.blockio_type = std::stoi(_len);
			argc = AdjustArgs(i, argv, argc, 1);
			goto restart;
		case IPC_T:
			snip.ipc_type = std::stoi(_len);
			argc = AdjustArgs(i, argv, argc, 1);
			goto restart;
		case FS_T:
			snip.fsapi_type = std::stoi(_len);
			argc = AdjustArgs(i, argv, argc, 1);
			goto restart;
		default:
			break;
		}
	}
	return argc;
}

/**
 * @brief para the args
 * @param  argc             desc
 * @param  argv             desc
 */
void 
Snip::ParaArgs(int argc, char* argv[]){
	if(argc < ARG_NUM){
		ErrUse(GetInstance());
		return;
	}
	if(GetInstance().snip_to_func.count(argv[1]) == 0){
		ErrUse(GetInstance());
		return;
	}
	if(argc > 2){
		/// 存在额外参数
		argc = ProcessOptArgs(argc, argv);
		if(argc != ARG_NUM){
			ErrUse(GetInstance());
			return;
		}
	}
	std::cout 
	<< "WELCOME TO SNIP!" << std::endl;
	Snip::snip_to_func[argv[1]]();  /// 执行目标函数
}

/**
 * @brief how to use snip
 */
void 
Snip::ShowUsage(){
	std::cout 
        << "First level: lwp, net, ipc, sm(sync and mutex), fs" << std::endl 
        << "Second Level: servert=/blockiot=/ipct=/fsapit=" << std::endl
        << "    server type=" << std::endl
        << "        0:multi processes" << std::endl
        << "        1:single process multi threads" << std::endl
        << "        2:select" << std::endl
        << "        3:poll" << std::endl
        << "        4:epoll" << std::endl
        << "    blockio type=" << std::endl
        << "        0:no blocking" << std::endl
        << "        1:blocking" << std::endl
        << "    ipc type=" << std::endl
        << "        0:pipe" << std::endl
        << "        1:named pipe" << std::endl
        << "        2:semaphore" << std::endl
        << "        3:signal" << std::endl
        << "        4:msg queue" << std::endl
        << "        5:shared memory" << std::endl
        << "        6:sockets" << std::endl
        << "    fs api type=" << std::endl
        << "        1:inotify" << std::endl
        << std::endl;
	return;
}

void
ErrUse(Snip& snip){
	std::cerr 
	<< "Error Usage! Please refer usage!" << std::endl;
	snip.ShowUsage();
	return;
}