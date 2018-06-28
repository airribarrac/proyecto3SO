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
						cv.wait(tlock);	//hace tuto
						//muere aqui
						//puts("asdasd");
							qmtx.lock();
						//saco tarea;
						auto tarea = tareas.front();
						tareas.pop();
						qmtx.unlock();
						disponibles--;
						//HACER TAREA
						tarea();
						qmtx.lock();
						while(disponibles==0 && !tareas.empty()){	//mientras todas estan ocupadas y hay tareas
							qmtx.unlock();
							auto tarea = tareas.front();
							tareas.pop();					//trato de hacer mientras quedan
							qmtx.lock();
						}
						qmtx.unlock();
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
		argumentos && ...args) -> std::packaged_task<MRtrn(void)> {
		auto aux = std::bind(std::forward<funcion>(func),
		std::forward<argumentos>(args)...	);
		qmtx.lock();
		tareas.push(aux);
		qmtx.unlock();
		cv.notify_one();
		return std::packaged_task<MRtrn(void)>(aux);
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
	vector<packaged_task<int()> > res;
	for(int i=0;i<5;i++){
		res.push_back(
			tp.encolar([i,tam,v]{
				cout<<"aloalo"<<endl;
				int mini = v[i];
				for(int j=1;j<tam/5;j++){
					mini = min(v[i+j],mini);
				}
				return mini;
			})
		);
	}
	puts("kk");
	int rmini = res[0].get_future().get();
	puts("asdas");
	for(int i=1;i<res.size();i++){
		rmini = min(res[i].get_future().get(),rmini);
	}
	cout<<"minimo es: "<<rmini<<endl;
	return 0;
}