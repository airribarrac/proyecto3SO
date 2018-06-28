#include <functional>
#include <bits/stdc++.h>
using namespace std;

class ThreadPool{
private:
	int size;
	atomic<int> disponibles;
	condition_variable cv;
	mutex mtx,qmtx;
	queue<function<void(void)>> tareas;
	void waitforwork(){
		unique_lock<mutex> lock(mtx);
		while(1){
			cv.wait(lock);	//hace tuto
			qmtx.lock();
			//saco tarea;
			auto tarea = tareas.front();
			tareas.pop();
			qmtx.unlock();
			disponibles--;
			//HACER TAREA
			tarea();
			disponibles++;
		}
	}

public:
	ThreadPool(int _size){
		size=_size;
		disponibles=size;
		for(int i=0;i<size;i++){
			thread t([this]{waitforwork();});
		}
	}
	template<
		typename funcion,
		typename ... argumentos,
		typename MRtrn=typename std::result_of<funcion(argumentos...)>::type>
	auto encolar(
		funcion && func,
		argumentos && ...args) -> std::packaged_task<MRtrn(void)> {
		auto aux = std::bind(std::forward<funcion>(func),
		std::forward<argumentos>(args)...	);
		tareas.push(aux);
		return std::packaged_task<MRtrn(void)>(aux);
	}
};


int main(){

}
