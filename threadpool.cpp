#include <functional>
#include <bits/stdc++.h>
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
		//puts("voy a crear");
		size=_size;
		disponibles=size;
		stopped=false;
		for(int i=0;i<size;i++){
			//puts("creo");
			hebras.push_back(thread (
				[this]{
					//puts("owo");
					while(1){
						//puts("alo");
						unique_lock<mutex> tlock(this->mtx);
						cv.wait(tlock);	//hace tuto
						//muere aqui
						//puts("asdasd");
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
							this->qmtx.unlock();
							auto tarea = this->tareas.front();
							this->tareas.pop();					//trato de hacer mientras quedan
							this->qmtx.lock();
						}
						if(this->stopped && this->tareas.empty()){
							//puts("F");
							return;
						}
						this->qmtx.unlock();
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
		//puts("aaaaaaaaaaaaaaaAAAAAAAAAAAAAAAAAAAA");
		cv.notify_all();
		for(int i=0;i<hebras.size();i++){
			hebras[i].join();
		}
	}
};


int main(){
	vector<int> v;
	puts("PROFE SOMOS UN DESASTRE Y NO PUDIMOS HACERLO :(");
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
		res.push_back(
			tp.encolar([i,tam,v]{
				cout<<"aloalo"<<endl;
				int mini = v[i];
				for(int j=1;j<tam/5;j++){
					mini = min(v[i+j],mini);
				}
				cout<<"termine"<<endl;
				return mini;
			})
		);
	}
	puts("kk");			
	tp.waitTodos();
	int rmini = res[0].get();
	puts("asdas");

	for(int i=1;i<res.size();i++){
		rmini = min(res[i].get(),rmini);
	}
	cout<<"minimo es: "<<rmini<<endl;
	return 0;
}
