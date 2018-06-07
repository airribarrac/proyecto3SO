#include <functional>

using namespace std;

class ThreadPool{
private:
	int size;
	atomic<int> disponibles;
	condition_variable cv;
	mutex mtx,qmtx;
	queue<function<void(void)>> tareas;
	auto waitforwork(){
		while(1){
			cv.wait(mtx);
			qmtx.lock();
			//saco tarea;
			auto tarea = tareas.front();
			tareas.pop();
			qmtx.unlock();

			disponibles--;
			disponibles++;
		}
	}

public:
	ThreadPool(int _size){
		size=_size;
		disponibles=size;
		for(int i=0;i<size;i++){
			thread t(waitforwork);
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
		return std::packaged_task<MRtrn(void)>(aux);
	}
};

