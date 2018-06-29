#include <functional>
#include <bits/stdc++.h>
#include <unistd.h>
using namespace std;

class ThreadPool{
private:
	int size;
	atomic<int> disponibles;
	atomic<bool> stopped;
	condition_variable cv;
	mutex mtx,qmtx;
	queue<function<void()>> tareas;
	vector<thread> hebras;

public:
	ThreadPool(int _size){
		size=_size;
		disponibles=size;
		stopped=false;
		for(int i=0;i<size;i++){
			hebras.push_back(thread (
				[this]{
					while(1){
						unique_lock<mutex> tlock(this->mtx);
						this->cv.wait(tlock);	//hace tuto
						puts("Hace tarea");
						this->qmtx.lock();
						//saco tarea;
						auto tarea = this->tareas.front();
						this->tareas.pop();
						this->qmtx.unlock();
						this->disponibles--;
						//HACER TAREA
						tarea();
						this->qmtx.lock();

						while(this->disponibles==0 && !this->tareas.empty()){	//mientras todas estan ocupadas y hay tareas
							puts("alo");
							this->qmtx.unlock();
							this->cv.wait(tlock);
							auto tarea = this->tareas.front();
							this->tareas.pop();					//trato de hacer mientras quedan
							this->qmtx.lock();
							tarea();
						}
						if(this->stopped && this->tareas.empty()){
							return;
						}
						this->qmtx.unlock();
						this->cv.notify_one();
						this->disponibles++;	//si no queda nada estoy libre
					}
				}
			));
			
		}
	}
	template<
		typename funcion,
		typename ... argumentos,
		typename MRtrn=typename std::result_of<funcion(argumentos...)>::type>
	auto encolar(
		funcion && func,
		argumentos && ...args) -> std::future<MRtrn> {
		auto tarea = make_shared<packaged_task<MRtrn()> >(
			bind(forward<funcion>(func),forward<argumentos>(args)...));
		future<MRtrn> result = tarea->get_future();
		qmtx.lock();
		tareas.push([tarea](){(*tarea)();});
		qmtx.unlock();
		cv.notify_one();
		return result;
	}
	template<
		typename funcion,
		typename ... argumentos,
		typename MRtrn=typename std::result_of<funcion(argumentos...)>::type>
	auto spawn(
		funcion && func,
		argumentos && ...args) -> std::future<MRtrn> {
		auto tarea = make_shared<packaged_task<MRtrn()> >(
			bind(forward<funcion>(func),forward<argumentos>(args)...));
		future<MRtrn> result = tarea->get_future();
		if(disponibles>0){
			qmtx.lock();
			tareas.push([tarea](){(*tarea)();});
			qmtx.unlock();
			cv.notify_one();
			return result;
		}else{
			tarea();
			return result;
		}	
	}
	void waitTodos(){
		stopped = true;
	    cv.notify_all();
	    for(thread &hilo: hebras)
	        hilo.join();
	}
	~ThreadPool(){
		waitTodos();
	}
};


int main(){
	vector<int> v;
	int tam = 25;
	for(int i=0;i<tam;i++){
		v.push_back(i);
	}
	random_shuffle(v.begin(), v.end());
	cout<<"vector a usar"<<endl;
	for(int i=0;i<tam;i++){
		cout<<v[i]<<" ";
	}
	cout<<endl;
	ThreadPool tp(50);
	vector<future<int> > res;
	for(int i=0;i<25;i++){
		res.push_back(
			tp.encolar([i,tam,v]{
				sleep(1);
				int num = v[i] * i;
				return num;
			})
		);
	}
	for(auto && result: res)
        cout << result.get() << endl;
	tp.waitTodos();
	return 0;
}
