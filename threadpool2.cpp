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
	vector<thread> hilos;
	void waitforwork(){
		
	}

public:
	ThreadPool(int _size){
		puts("voy a crear");
		size=_size;
		disponibles=size;
		for(int i=0;i<size;i++){
			puts("creo");
			hilos.emplace_back(
				[this]{
					puts("owo");
					while(1){
						//puts("alo");
						unique_lock<mutex> tlock(this->mtx);
						this->cv.wait(tlock);	//hace tuto
						//muere aqui
						//puts("asdasd");
						this->qmtx.lock();
						//saco tarea;
						auto tarea = move(this->tareas.front());
						this->tareas.pop();
						this->qmtx.unlock();
						disponibles--;
						//HACER TAREA
						tarea();
						this->qmtx.lock();
						while(disponibles==0 && !tareas.empty()){	//mientras todas estan ocupadas y hay tareas
							this->qmtx.unlock();
							auto tarea = move(this->tareas.front());
							this->tareas.pop();					//trato de hacer mientras quedan
							this->qmtx.lock();
						}
						this->qmtx.unlock();
						disponibles++;	//si no queda nada estoy libre
					}
				}
			);
			//t.detach();
		}
	}
	template<
		typename funcion,
		typename ... argumentos,
		typename MRtrn=typename std::result_of<funcion(argumentos...)>::type>
	auto encolar(
		funcion && func,
		argumentos && ...args) -> std::future<MRtrn> {
		auto aux = std::bind(std::forward<funcion>(func),
		std::forward<argumentos>(args)...	);
		qmtx.lock();
		tareas.push(aux);
		qmtx.unlock();
		cv.notify_one();
		auto tarea = make_shared<packaged_task<MRtrn()> >(aux);
		future<MRtrn> res = tarea->get_future();
		return res;
	}
	template<
		typename funcion,
		typename ... argumentos,
		typename MRtrn=typename std::result_of<funcion(argumentos...)>::type>
	auto spawn(
		funcion && func,
		argumentos && ...args) -> std::packaged_task<MRtrn(void)> {
		auto aux = std::bind(std::forward<funcion>(func),
		std::forward<argumentos>(args)...	);
		if(disponibles>0){
			qmtx.lock();
			tareas.push(aux);
			qmtx.unlock();
			cv.notify_one();
			return std::packaged_task<MRtrn(void)>(aux);
		}else{
			aux();
			return std::packaged_task<MRtrn(void)>(aux);
		}	
	}
	void waitTodos(){
	    unique_lock<mutex> lock(mtx);
	    cv.notify_all();
	    for(thread &hilo: hilos)
	        hilo.join();
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
	ThreadPool tp(5);
	vector<future<int> > res;
	for(int i=0;i<5;i++){
		res.emplace_back(
			tp.encolar([i,v]{
				cout<<"aloalo"<<endl;
				
				return v[i];
			})
		);
	}
	puts("kk");
	for(auto && result: res)
        std::cout << result.get() << ' ';
    std::cout << std::endl;
    tp.waitTodos();
	return 0;
}