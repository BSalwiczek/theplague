#include <iostream>
#include <vector>
#include <tuple>
#include <string>
#include <map>
#include <queue>

using namespace std;

class Vertex;
class Worker;

class Edge
{
public:
	Worker* coresponding_worker;

	Vertex* source;
	Vertex* sink;
	Edge* reverse;
	bool residual;
	int flow_limit;
	int residual_flow;
	Edge(Vertex* source, Vertex* sink, bool residual, int fl, int rf, Worker* cw = nullptr) : source(source), sink(sink), residual(residual), flow_limit(fl), residual_flow(rf), coresponding_worker(cw) { reverse = nullptr; }
	void setReverse(Edge* rev)
	{
		reverse = rev;
	}
};

class Vertex
{
public:
	bool active;
	string name;
	vector<Edge*> edges;
	Vertex(string name = "")
	{
		this->name = name;
		active = true;
	}
	void addEdge(Edge* edge)
	{
		edges.push_back(edge);
	}

	void showEdges()
	{
		for (int i = 0; i < edges.size(); i++)
		{
			if (!edges[i]->residual)
			{

				cout << "Edge from " << name << " to " << edges[i]->sink->name <<", flow = " << edges[i]->flow_limit-edges[i]->residual_flow<< endl;
				edges[i]->sink->showEdges();
			}
		}
	}
};

class Worker
{
public:
	Vertex* coresponding_vertex;
	string department_name;
	string name;
	int id;
	int value;
	int all_hours;
	int hours_per_day;
	Worker(Vertex* c, string d, string n, int v, int a, int h, int id) : coresponding_vertex(c), department_name(d), name(n), value(v), all_hours(a), hours_per_day(h), id(id) {}
};

class Task
{
public:
	Vertex* coresponding_vertex;
	string group_name;
	int day;
	int time;
	int min_man_hours;
	int max_man_hours;
	Task(Vertex* c,string g, int d, int t, int mi, int ma) : coresponding_vertex(c), group_name(g), day(d), time(t), min_man_hours(mi), max_man_hours(ma) {}
};

void addDaysAndTasks(map<int, vector<Task*>>& tasks, Vertex* source, vector<Worker*>& workers)
{
	vector<Vertex*> days_vertexes;
	for (int i = 0; i < source->edges.size(); i++)
	{
		for (const auto& task : tasks)
		{
			//Vertex* day_vertex = new Vertex(workers[i]->name + "d" + to_string(task.first));
			Vertex* day_vertex = new Vertex(to_string(task.first));
			days_vertexes.push_back(day_vertex);
			int index = 1;

			map<int, Vertex*> times;
			Edge* e1;
			Edge* e2;
			for (Task* t : task.second) // tasks for that day (task.first)
			{
				Vertex* time_vertex;
				if (times[t->time] == nullptr) // check if there is no other task on that day on that hour
				{
					//time_vertex = new Vertex(day_vertex->name + "time" + to_string(t->time));
					time_vertex = new Vertex(to_string(t->time));

					e1 = new Edge(day_vertex, time_vertex, false, 1, 1);
					day_vertex->addEdge(e1);
					e2 = new Edge(time_vertex, day_vertex, true, 1, 0);
					time_vertex->addEdge(e2); // residual edge
					e1->setReverse(e2);
					e2->setReverse(e1);

					times[t->time] = time_vertex;
				}
				else
					time_vertex = times[t->time];

				e1 = new Edge(time_vertex, t->coresponding_vertex, false, 1, 1);
				time_vertex->addEdge(e1);
				e2 = new Edge(t->coresponding_vertex, time_vertex, true, 1, 0);
				t->coresponding_vertex->addEdge(e2); // residual edge
				e1->setReverse(e2);
				e2->setReverse(e1);
				index++;
			}
		}
		for (Vertex* day : days_vertexes)
		{
			Vertex* worker_vertex = source->edges[i]->sink;
			Edge* e1 = new Edge(worker_vertex,day, false, workers[i]->hours_per_day, workers[i]->hours_per_day);
			worker_vertex->addEdge(e1);
			Edge* e2 = new Edge(day, worker_vertex, true, workers[i]->hours_per_day, 0);
			day->addEdge(e2); // residual edge

			e1->setReverse(e2);
			e2->setReverse(e1);
		}
		days_vertexes.clear();
	}
}

map<Vertex*, Edge*> bfs(Vertex* source)
{
	queue<Vertex*> q;
	q.push(source);
	
	map<Vertex*, Edge*> pred;
	while (!q.empty())
	{
		Vertex* curr_vertex = q.front();
		q.pop();

		for (Edge* e : curr_vertex->edges)
		{
			if (pred[e->sink] == nullptr && e->sink != source && e->sink->active && e->residual_flow > 0)
			{
				pred[e->sink] = e;
				q.push(e->sink);
			}
		}
	}
	return pred;
}

int edmondsKarp(Vertex* source, Vertex* sink)
{
	int flow = 0;
	map<Vertex*, Edge*> pred = bfs(source);
	while (pred[sink] != nullptr)
	{
		int df = 1;
		for (Edge* e = pred[sink]; e != nullptr; e = pred[e->source])
		{

			e->residual_flow -= df;
			e->reverse->residual_flow += df;
			//e->reverse->true_flow += df;
		}
		flow += df;
		pred = bfs(source);
	}
	return flow;
}

void resetFlowGraph(Vertex* source)
{
	for (Edge* edge : source->edges)
	{
		if (!edge->residual && edge->residual_flow != edge->flow_limit)
		{
			edge->residual_flow = edge->flow_limit;
			edge->reverse->residual_flow = 0;
			resetFlowGraph(edge->sink);
		}
	}
}

int min(int x, int y)
{
	return x > y ? y : x;
}

void evaluateSchedule(bool* optimal_workers_activity, int required_flow, int& min_value, bool* workers_activity, Vertex* source, Vertex* sink)
{
	int flow = edmondsKarp(source, sink);
	if (flow >= required_flow)
	{
		int current_value = 0;
		for (Edge* e : source->edges)
		{
			if (e->sink->active)
			{
				current_value += e->coresponding_worker->value;
			}
		}
		if (current_value < min_value)
		{
			for (int i = 0; i < source->edges.size(); i++)
			{
				optimal_workers_activity[i] = workers_activity[i];
			}
			min_value = current_value;
		}
	}
	resetFlowGraph(source);
}

void fairSchedule(bool* optimal_workers_activity, int required_flow, int& min_value, bool* workers_activity, Vertex* source, Vertex* sink, vector<vector<Worker*>> departments, int current_dep, int max_workers_taken)
{
	/*
		Complexity: O(2^k) (k - number of departments)
	*/
	if (current_dep == departments.size())
	{
		evaluateSchedule(optimal_workers_activity, required_flow, min_value, workers_activity, source, sink);
		return;
	}

	for (int i = max_workers_taken-1; i <= min(max_workers_taken,departments[current_dep].size()); i++) // i - number of first workers taken (active) from department of id current_dep
	{
		for (int j = 0; j < departments[current_dep].size(); j++)
		{
			if (j >= i)
			{
				workers_activity[departments[current_dep][j]->id] = false;
				departments[current_dep][j]->coresponding_vertex->active = false;
			}
			else
			{
				workers_activity[departments[current_dep][j]->id] = true;
				departments[current_dep][j]->coresponding_vertex->active = true;
			}
		}

		fairSchedule(optimal_workers_activity, required_flow, min_value, workers_activity, source, sink, departments, current_dep + 1, max_workers_taken); // moving to next deparment

	}
}

void listSchedule(bool* optimal_workers_activity, int required_flow, int& min_value, bool* workers_activity, Vertex* source, Vertex* sink, vector<vector<Worker*>> departments, int current_dep)
{
	/*
		Complexity: O(n^k) (n - max amount of workers in department, k - number of departments)
	*/

	if (current_dep == departments.size())
	{
		evaluateSchedule(optimal_workers_activity, required_flow, min_value, workers_activity, source, sink);
		return;
	}
	for (int i = 0; i <= departments[current_dep].size(); i++) // i - number of first workers taken (active) from department of id current_dep
	{
		for (int j = 0; j < departments[current_dep].size(); j++)
		{
			if (j >= i)
			{
				workers_activity[departments[current_dep][j]->id] = false;
				departments[current_dep][j]->coresponding_vertex->active = false;
			}
			else
			{
				workers_activity[departments[current_dep][j]->id] = true;
				departments[current_dep][j]->coresponding_vertex->active = true;
			}
		}

		listSchedule(optimal_workers_activity, required_flow, min_value, workers_activity, source, sink, departments, current_dep + 1); // moving to next department
	}
}

void writeAnswer(vector<Worker*> workers, Vertex* source, int output_option)
{
	map<string, int> dep_amount;
	int total_value = 0;
	for (Edge* e : source->edges)
	{
		if (e->sink->active)
		{
			total_value += e->coresponding_worker->value;
			dep_amount[e->coresponding_worker->department_name] += 1;
		}
	}
	cout << total_value << endl;

	if (output_option != 0)
	{
		for (auto p : dep_amount)
		{
			cout << p.first << " " << p.second << endl;
		}

		if (output_option == 2)
		{
			for (Worker* worker : workers)
			{
				int sum_of_performed_tasks = 0;
				for (Edge* day : worker->coresponding_vertex->edges)
				{
					if (!day->residual)
					{
						for (Edge* e : day->sink->edges)
						{
							if (!e->residual && e->flow_limit - e->residual_flow > 0)
							{
								sum_of_performed_tasks++;
							}
						}
					}
				}

				if (sum_of_performed_tasks > 0)
				{
					cout << worker->name << " " << sum_of_performed_tasks << endl;
				}

				for (Edge* day : worker->coresponding_vertex->edges)
				{
					if (!day->residual)
					{
						for (Edge* e : day->sink->edges)
						{
							if (!e->residual && e->flow_limit - e->residual_flow > 0)
							{
								sum_of_performed_tasks++;
								cout << e->sink->edges[1]->sink->edges[0]->sink->name << " " << day->sink->name << " " << e->sink->name << endl;
							}
						}
					}
				}

			}
		}
	}
}

void deleteGraph(map<Vertex*,bool>& deleted, Vertex* source)
{
	for (int i=0;i<source->edges.size();i++)
	{
		if (!source->edges[i]->residual)
			deleteGraph(deleted, source->edges[i]->sink);
		delete source->edges[i];
	}
	if(!deleted[source])
		delete source;
	deleted[source] = true;
}

int main()
{
	Vertex* source = new Vertex("s");
	Vertex* sink = new Vertex("t");

	int scheduling_mode, output_option, number_of_departments;
	cin >> scheduling_mode;
	int k;
	if (scheduling_mode == 3)
	{
		cin >> k;
	}
	cin >> output_option >> number_of_departments;
	
	vector<Worker*> workers;

	string department_name, worker_name;
	int num_of_workers, value, all_hours, hours_per_day;

	vector<vector<Worker*>> departments; // departments[i] = list of workers of i-th department

	int worker_id = 0;
	vector<Worker*> dep_workers;
	for (int i = 0; i < number_of_departments; i++)
	{
		cin >> department_name >> num_of_workers;
		for (int j = 0; j < num_of_workers; j++)
		{
			cin >> worker_name >> value >> all_hours >> hours_per_day;

			Vertex* worker_vertex = new Vertex(worker_name);

			Worker* worker = new Worker(worker_vertex, department_name, worker_name, value, all_hours, hours_per_day, worker_id);
			dep_workers.push_back(worker);
			worker_id++;

			workers.push_back(worker);

			Edge* e1 = new Edge(source, worker_vertex, false, all_hours, all_hours, worker);
			source->addEdge(e1);
			Edge* e2 = new Edge(worker_vertex, source, true, all_hours, 0);
			worker_vertex->addEdge(e2); // residual edge

			e1->setReverse(e2);
			e2->setReverse(e1);
		}
		departments.push_back(dep_workers);
		dep_workers.clear();
	}

	int num_of_groups, num_of_tasks_in_group, all_man_hours_needed;
	string task_group_name;

	map<int, vector<Task*>> tasks; // task[i] = vector of tasks for i-th day

	int required_flow = 0;

	cin >> num_of_groups;
	for (int i = 0; i < num_of_groups; i++)
	{
		cin >> task_group_name >> num_of_tasks_in_group >> all_man_hours_needed;
		Vertex* group_vertex = new Vertex(task_group_name);

		Edge* e1;
		Edge* e2;
		int day, time, min_man_hours, max_man_hours;
		for (int j = 0; j < num_of_tasks_in_group; j++)
		{
			cin >> day >> time >> min_man_hours >> max_man_hours;

			all_man_hours_needed -= min_man_hours;
			required_flow += min_man_hours;

			Vertex* task_vertex = new Vertex("T"+to_string(j+1));

			e1 = new Edge(task_vertex, group_vertex, false, max_man_hours - min_man_hours, max_man_hours - min_man_hours);
			task_vertex->addEdge(e1);
			e2 = new Edge(group_vertex, task_vertex, true, max_man_hours - min_man_hours, 0);
			group_vertex->addEdge(e2); // residual edge

			e1->setReverse(e2);
			e2->setReverse(e1);

			e1 = new Edge(task_vertex, sink, false, min_man_hours, min_man_hours);
			task_vertex->addEdge(e1);
			e2 = new Edge(sink, task_vertex, true, min_man_hours, 0);
			sink->addEdge(e2); // residual edge

			e1->setReverse(e2);
			e2->setReverse(e1);

			tasks[day].push_back(new Task(task_vertex,task_group_name, day, time, min_man_hours, max_man_hours));
		}

		required_flow += all_man_hours_needed;

		e1 = new Edge(group_vertex, sink, false, all_man_hours_needed, all_man_hours_needed);
		group_vertex->addEdge(e1);

		e2 = new Edge(sink, group_vertex, true, all_man_hours_needed, 0);
		sink->addEdge(e2); // residual edge

		e1->setReverse(e2);
		e2->setReverse(e1);
	}

	addDaysAndTasks(tasks, source, workers);


	bool* optimal_workers_activity = new bool[sink->edges.size()]{ true };
	bool* workers_activity = new bool[sink->edges.size()]{ true };
	int min_value = INT_MAX;
	
	//scheduling_mode = 2;
	if (scheduling_mode == 1)
	{
		//cout << "LIST" << endl;
		listSchedule(optimal_workers_activity, required_flow, min_value, workers_activity, source, sink, departments, 0);
	}
	else if (scheduling_mode == 2)
	{
		//cout << "FAIR" << endl;
		int max_workers = 0;
		for (auto workers : departments)
		{
			max_workers = max_workers > workers.size() ? max_workers : workers.size();
		}

		/*
			complexity: O(n*2^k) - (n - max wokers in department, k - num of departments)
		*/
		for (int i = 1; i < max_workers; i++)
		{
			fairSchedule(optimal_workers_activity, required_flow, min_value, workers_activity, source, sink, departments, 0, i);
		}
	}

	resetFlowGraph(source);
	int index = 0;
	for (Edge* e : source->edges)
	{
		e->sink->active = optimal_workers_activity[index];
		index++;
	}
	edmondsKarp(source, sink);

	writeAnswer(workers, source, output_option);
	

	delete[] optimal_workers_activity;
	delete[] workers_activity;
	departments.clear();
	tasks.clear();
	for (auto& worker : workers)
	{
		delete worker;
	}
	workers.clear();
	map<Vertex*, bool> deleted;
	deleteGraph(deleted, source);
	deleted.clear();

	int x;
	cin >> x;
	return 0;
}
