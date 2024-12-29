#include <iostream>
#include <algorithm>
#include <queue>
#include <vector>
#include <ctime>
#include <set>
#include <map>
#include <iomanip>

// 常量定义
#define MAX_PROCESSES 100
#define MAX_QUEUES 5

// PCB结构体
struct PCB
{
	int id;                          // 进程ID
	int arrival;                     // 到达时间
	int burst;                       // 运行时间
	int remaining;                   // 剩余运行时间
	int start_time;                  // 开始执行时间
	int finish_time;                 // 完成时间
	int turnaround_time;             // 周转时间
	double weighted_turnaround_time; // 带权周转时间
};

// 全局变量
int num_queues;                              // 就绪队列数量
int num_processes;                           // 进程数量
int time_quantum[MAX_QUEUES];                // 每个队列的时间片大小
std::queue<PCB> ready_queues[MAX_QUEUES];    // 多级就绪队列
std::vector<PCB> all_processes;              // 所有进程信息
std::map<int, int> process_map;              // 进程ID到索引的映射
int total_turnaround_time = 0;               // 周转总时间
double total_weighted_turnaround_time = 0.0; // 带权周转总时间

// 函数声明
void initialize(int option);
void executeMFQ();
void displayResults();
void displayProcesses();

// 主函数
int main()
{
	srand(time(0));
	while (true)
	{
		std::cout << "请选择测试样例生成方式：\n1. 随机生成\n2. 键盘输入\n3. 退出\n";
		int choice;
		std::cin >> choice;
		
		if (choice == 1)
		{
			std::cout << "随机生成: \n";
			initialize(1);
		}
		else if (choice == 2)
		{
			std::cout << "键盘输入: \n";
			initialize(2);
		}
		else
		{
			break;
		}
		
		executeMFQ();
		displayResults();
	}
	return 0;
}

// 展示进程信息
void displayProcesses()
{
	std::cout << "就绪队列总数：" << num_queues << '\n';
	std::cout << "第一个就绪队列时间片大小：" << time_quantum[0] << '\n';
	std::cout << "进程总数：" << num_processes << '\n';
	for (const auto &pcb : all_processes)
	{
		std::cout << "进程 " << pcb.id
		<< " - 到达时间：" << pcb.arrival
		<< "，运行时间：" << pcb.burst << '\n';
	}
}

// 初始化函数
void initialize(int option)
{
	std::set<int> arrival_times; // 用于避免重复到达时间
	
	if (option == 1)
	{ // 随机生成
		num_queues = rand() % 5 + 1;
		time_quantum[0] = rand() % 10 + 1;
		for (int i = 1; i < num_queues; ++i)
			time_quantum[i] = time_quantum[i - 1] * 2;
		
		num_processes = rand() % 10 + 1;
		all_processes.resize(num_processes);
		for (int i = 0; i < num_processes; ++i)
		{
			PCB &pcb = all_processes[i];
			pcb.id = i + 1;
			pcb.arrival = rand() % 20 + 1;
			while (arrival_times.count(pcb.arrival))
				pcb.arrival = rand() % 20 + 1;
			arrival_times.insert(pcb.arrival);
			pcb.burst = rand() % 50 + 1;
			pcb.remaining = pcb.burst;
			pcb.start_time = pcb.finish_time = -1;
			pcb.turnaround_time = -1;
			pcb.weighted_turnaround_time = -1;
		}
	}
	else
	{ // 键盘输入
		std::cout << "请输入就绪队列数量(最大为5): ";
		std::cin >> num_queues;
		std::cout << "请输入第一个就绪队列时间片大小: ";
		std::cin >> time_quantum[0];
		for (int i = 1; i < num_queues; ++i)
			time_quantum[i] = time_quantum[i - 1] * 2;
		
		std::cout << "请输入进程数量: ";
		std::cin >> num_processes;
		all_processes.resize(num_processes);
		for (int i = 0; i < num_processes; ++i)
		{
			PCB &pcb = all_processes[i];
			pcb.id = i + 1;
			std::cout << "请输入第 " << pcb.id << " 个进程的到达时间: ";
			std::cin >> pcb.arrival;
			while (arrival_times.count(pcb.arrival))
			{
				std::cout << "到达时间重复，请重新输入: ";
				std::cin >> pcb.arrival;
			}
			arrival_times.insert(pcb.arrival);
			std::cout << "请输入第 " << pcb.id << " 个进程的运行时间: ";
			std::cin >> pcb.burst;
			pcb.remaining = pcb.burst;
			pcb.start_time = pcb.finish_time = -1;
			pcb.turnaround_time = -1;
			pcb.weighted_turnaround_time = -1;
		}
	}
	
	std::sort(all_processes.begin(), all_processes.end(),
		[](const PCB &a, const PCB &b)
		{ return a.arrival < b.arrival; });
	
	for (int i = 0; i < num_processes; ++i)
		process_map[all_processes[i].id] = i;
	
	displayProcesses();
}

// 多级反馈队列调度算法
void executeMFQ()
{
	int current_queue = 0;                       // 当前所在队列
	int current_process = 0;                     // 当前进程
	int current_time = all_processes[0].arrival; // 当前时间
	int process_running = 1;                     // 当前正在进行的进程数
	
	// 将下一个进程加入就绪队列
	auto nextProcess = [&]()
	{
		current_process++;
		ready_queues[0].push(all_processes[current_process]);
		current_time = all_processes[current_process].arrival;
		current_queue = 0;
		process_running++;
	};
	
	// 将第一个进程放入第一个队列
	ready_queues[0].push(all_processes[0]);
	
	while (true)
	{
		if (process_running == 0)
		{
			if (current_process == num_processes - 1)
				break; // 所有进程已运行完毕
			else
				nextProcess(); // 将下一个进程加入就绪队列
		}
		
		while (!ready_queues[current_queue].empty())
		{
			PCB now = ready_queues[current_queue].front();
			ready_queues[current_queue].pop();
			
			int tmp = time_quantum[current_queue];
			if (current_process + 1 < num_processes)
				tmp = std::min(tmp, all_processes[current_process + 1].arrival - current_time);
			
			if (now.remaining <= tmp)
			{ // 当前进程先运行完
				if (now.start_time == -1)
					now.start_time = current_time; // 记录开始时间
				current_time += now.remaining;
				all_processes[process_map[now.id]].finish_time = current_time;
				process_running--;
			}
			else if (current_process + 1 >= num_processes ||
				(time_quantum[current_queue] <= all_processes[current_process + 1].arrival - current_time))
			{
				// 时间片已用完
				if (now.start_time == -1)
					now.start_time = current_time; // 记录开始时间
				current_time += time_quantum[current_queue];
				now.remaining -= time_quantum[current_queue];
				if (current_queue == num_queues - 1)
					ready_queues[current_queue].push(now); // 最后一级队列RR
				else
					ready_queues[current_queue + 1].push(now); // FCFS
			}
			else
			{ // 时间片未用完，且下一个进程先到
				if (now.start_time == -1)
					now.start_time = current_time; // 记录开始时间
				now.remaining -= (all_processes[current_process + 1].arrival - current_time);
				ready_queues[current_queue].push(now);
				nextProcess(); // 将下一个进程加入就绪队列
			}
		}
		
		if (current_queue + 1 < num_queues)
			current_queue++;
	}
}

// 打印调度结果
void displayResults()
{
	std::cout << std::fixed << std::setprecision(2);
	std::cout << "\n进程调度结果:\n";
	std::cout << "进程ID\t开始时间\t运行时间\t完成时间\t周转时间\t带权周转时间\n";
	
	for (int j = 0; j < num_processes; ++j)
	{
		int i = process_map[j + 1];
		all_processes[i].turnaround_time = all_processes[i].finish_time - all_processes[i].arrival;
		all_processes[i].weighted_turnaround_time = 1.0 * all_processes[i].turnaround_time / all_processes[i].burst;
		total_turnaround_time += all_processes[i].turnaround_time;
		total_weighted_turnaround_time += all_processes[i].weighted_turnaround_time;
		
		std::cout << all_processes[i].id << "\t"
		<< all_processes[i].arrival << "\t\t"
		<< all_processes[i].burst << "\t\t"
		<< all_processes[i].finish_time << "\t\t"
		<< all_processes[i].turnaround_time << "\t\t"
		<< all_processes[i].weighted_turnaround_time << "\n";
	}
	
	std::cout << "\n平均周转时间: " << (total_turnaround_time / (double)num_processes) << '\n';
	std::cout << "平均带权周转时间: " << (total_weighted_turnaround_time / (double)num_processes) << '\n';
}

