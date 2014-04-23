#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include<math.h>
#include <list>
#include <stdio.h>
#include<ctime>
using namespace std;

const int K_NEIGHBORS=30;        //ITERM������ھ���
typedef map<int, double> RatingMap;//����һ������map



//����һ��User�ṹ�������û�ƽ�����֣��������ĵ�Ӱmap
struct User
{
	double average_rating;
	RatingMap rating_map;
};

//����һ��Film�ṹ��������Ӱƽ�����֣��������ĵ�Ӱmap
struct Film 
{
	double average_rating;
	RatingMap rating_map;
};

//����һ��TestUser�ṹ���ڲ��Լ���ʹ�ã������û�id����Ӱid����ʵ���֣�Ԥ������
struct TestUser
{
	int user_id;
	int film_id;
	double real_rating;
	double predict_rating;

};

//����һ��SimilarFilm ������ŵ�Ӱ�����Ƶ�Ӱ���Ѿ����ƶ�
struct SimilarFilm
{
	Film film;
	double similarity;
};

typedef map<int ,Film> FilmMap;   //����û���mapģ����
typedef list<TestUser>TestList;   //��Ų��Ե�listģ����
typedef map<int,User>UserMap;    //����û���mapģ����

//����������Ӱ�����ƶȺ���
double filmSimilarity(Film test_film, Film train_film);
//Ԥ���Ӱ�����ֺ���
double predictRating(Film test_film, int user_id, SimilarFilm *similar_film, int num);


int main()
{
	//����ʱ��
	time_t begin,end;
	begin=clock();

	FilmMap filmMap;
	UserMap userMap;

	//��ʽ��ȡѵ���ļ�
	ifstream train_file("train.txt");
	if(!train_file)
	{
        cerr<<"error:unable to open  file "<<"train.txt"<<endl;
        return -1;
    }
	string line;
	FilmMap::iterator filmItem;
	UserMap::iterator userItem;
	int uid = 0;
	int fid = 0;
	double rating = 0;

	//����ȡ�������� �ֱ��ŵ� UserMap ��filmMap��
    while(getline(train_file,line))
	{
		uid = fid = rating = 0;
        string str1,str2,str3;
        istringstream strstm(line);
        strstm>>str1>>str2>>str3;
        uid = atoi(str1.c_str());
        fid=atoi(str2.c_str());
        rating=atof(str3.c_str());
        filmItem = filmMap.find(fid);
		userItem = userMap.find(uid);
		if(filmItem == filmMap.end())
		{
			Film film_temp;
			film_temp.average_rating = 0;
			film_temp.rating_map.insert(pair<int,double>(uid,rating));
			filmMap.insert(pair<int,Film>(fid,film_temp));
		}
		else
		{
			filmItem->second.rating_map.insert(pair<int,double>(uid,rating));
		}
		
		if(userItem == userMap.end())
		{
			
			User user_temp;
			user_temp.average_rating = 0;
			user_temp.rating_map.insert(pair<int,double>(fid,rating));
			userMap.insert(pair<int,User>(uid,user_temp));
		}
		else
		{
			userItem->second.rating_map.insert(pair<int,double>(fid,rating));
		}

        line.clear();
    }
	cout<<"Read file complete."<<endl;
    train_file.close();


	int count= 0;
	double sum = 0;
	RatingMap::iterator test_rating;
	RatingMap::iterator test_end;

	//�ֱ����UserMap��filmMap�������ÿ����Ӱ��ÿ���û���ƽ������
	for(userItem = userMap.begin(); userItem !=userMap.end(); userItem++)
	{

		count = 0;
		sum = 0;
		test_rating = userItem->second.rating_map.begin();
		test_end = userItem->second.rating_map.end();
		for (; test_rating != test_end ; test_rating++)
		{
			sum += test_rating->second;
			count++;
		}
		userItem->second.average_rating = sum/count;
	}

	for(filmItem = filmMap.begin(); filmItem !=filmMap.end(); filmItem++)
	{
		count = 0;
		sum = 0;

		test_rating = filmItem->second.rating_map.begin();
		test_end = filmItem->second.rating_map.end();

		for (; test_rating != test_end; test_rating++)
		{
			sum += test_rating->second;
			count++;
		}
		filmItem->second.average_rating = sum/count;
	}
	
	cout<<"Process data complete."<<endl;

	//��ȡtest�ļ�
	ifstream test_file("test.txt");
	TestList testList;
	TestUser test_user;

	if(!test_file)
	{
        cerr<<"error:unable to open  file "<<"test.txt"<<endl;
        return -1;
    }
	string test_line;
    while(getline(test_file,test_line))
	{
		uid = fid = rating = 0;
        string str4,str5,str6;
        istringstream str_stem(test_line);
        str_stem>>str4>>str5>>str6;
        uid =atoi(str4.c_str());
        fid=atoi(str5.c_str());
        rating=atof(str6.c_str());
        test_user.user_id = uid;
		test_user.film_id = fid;
		test_user.real_rating = rating;
		testList.push_back(test_user);
		test_line.clear();
	}
	test_file.close();

	cout<<"Load TestSet Complete."<<endl;
	FilmMap::iterator trainFilmIt;

	int min = 0;
	int flag = 0;
	double similarity  = 0;

	SimilarFilm similarFilm[K_NEIGHBORS] = {0};
	TestList::iterator test_item = testList.begin();


	for (; test_item != testList.end(); test_item++)
	{
		filmItem = filmMap.find(test_item->film_id);
		userItem = userMap.find(test_item->user_id);
		if(filmItem != filmMap.end())		
		{
			//��Ӱ����,�û�����
			if(userItem != userMap.end())
			{
				//��ֹ�û������ĵ�Ӱ��̫��
				if(userItem->second.rating_map.size()>2)
				{
					
					count = min = flag = 0;
					similarity = 0;
					test_rating = userItem->second.rating_map.begin();
					test_end = userItem->second.rating_map.end();
					//���û������ĵ�Ӱ���ҳ� �����ڵĵ�Ӱ
					for(; test_rating != test_end;test_rating++)
					{
						trainFilmIt = filmMap.find(test_rating->first);
						similarity = filmSimilarity((filmItem->second), (trainFilmIt->second));
						if(similarity>0)
						{
							if(count< K_NEIGHBORS)
							{
								similarFilm[count].film = trainFilmIt->second;
								similarFilm[count].similarity = similarity;
								count++;
							}
							else
							{
								if(flag ==0)
								{
									for (int i=1; i<count; i++)
									{
										if (similarFilm[i].similarity < similarFilm[min].similarity)
										{
											min = i;
										}	
									}
									flag = 1;
								}
								else
								{
									if (similarity > similarFilm[min].similarity)
									{
										similarFilm[min].film = trainFilmIt->second;
										similarFilm[min].similarity = similarity;
									}
								}
							}
						}
					}
					test_item->predict_rating = predictRating((filmItem->second), (test_item->user_id), similarFilm, count);
				
				}
				//���û������ĵ�ӰС��2��ʱ��ȡ��Ӱ��ƽ���ֺ��û���ƽ���ֵ�ƽ��ΪԤ������
				else 
				{
					test_item->predict_rating = int((filmItem->second.average_rating + userItem->second.average_rating)/2);
				}
			}
			//��Ӱ���ڣ��û�������
			else 
			{
				test_item->predict_rating = filmItem->second.average_rating;
			}
		}
		else
		{
			//��Ӱ�����ڣ��û�����
			if(userItem != userMap.end())			
			{
				test_item->predict_rating = userItem->second.average_rating ;

			}
			//�û������ڣ���ӰҲ������ʱ
			else								
			{
				test_item->predict_rating = 3.5;
			}
		}
}


		
	cout<<" Predict over."<<endl;


	ofstream output("output.txt");

	double MAE = 0;
	double RMSE = 0;

	output<<"�û�\t"<<"��Ӱ\t"<<"ʵ������\t"<<"Ԥ������\t"<<endl;

	test_item = testList.begin();
	//������ϣ���ʼ�����������ݣ������й���
	for (; test_item != testList.end(); test_item++)
	{

		MAE += fabs(test_item->predict_rating - test_item->real_rating);
		RMSE += pow((test_item->predict_rating - test_item->real_rating),2);
		output<<test_item->user_id<<"\t"<<test_item->film_id<<"\t"<<test_item->real_rating<<"\t"<<"             "<<test_item->predict_rating<<endl;
	}

	MAE /= testList.size();
	RMSE = sqrt(RMSE/testList.size());

	end  = clock();
	output<<"\nMAE: "<<MAE<<"\n"<<"RMSE: "<<RMSE<<"\n"<<"time: "<<double(end-begin)/CLOCKS_PER_SEC<<endl;
	

	cout<<"Output finish!"<<endl;
	return 0;


}

//����������Ӱ�����ƶȺ���
double filmSimilarity(Film test_film, Film train_film)
{
	int sum = test_film.rating_map.size();
	double *test_rating = new double[sum];
	double *train_rating = new double[sum];

	double test_rating_sum = 0;
	double train_rating_sum = 0;
	double test_average = 0;
	double train_average = 0;

	RatingMap::iterator test_user = test_film.rating_map.begin();
	RatingMap::iterator train_user = train_film.rating_map.begin();
	

	int index = 0;
	int test_size = test_film.rating_map.size();
	int train_size = train_film.rating_map.size();
	int test=0;
	int train = 0;
	//�ҵ�������Ӱ���������û��Լ�����
	for(;test<test_size &&train<train_size;)
	{
		if(test_user->first == train_user->first)
		{
			test_rating[index] = test_user->second;
			train_rating[index] = train_user->second;
			test_rating_sum += test_user->second;
			train_rating_sum += train_user->second;
			
			test_user++;
			train_user++;
			index++;
			test++;
			train++;
		}
		else if(test_user->first < train_user->first)
		{
			test_user++;
			test++;
		}
		else
		{
			train_user++;
			train++;
		}
	}
	
	test_average = test_rating_sum / (index+1);
	train_average = train_rating_sum / (index+1);

	double person1 = 0;
	double person2 = 0;
	double person3 = 0;

	//�������ƶ�
	for(int i=0; i<index; i++)
	{
		person1 += (test_rating[i] - test_average) * (train_rating[i] - train_average);
		person2 += pow((train_rating[i] - train_average),2);
		person3 += pow((train_rating[i] - train_average),2);
	}

	delete []test_rating;
	delete []train_rating;
	if (fabs(person2) <= 0.00001 || fabs(person3) <= 0.00001){
		return 0;
	}
	return person1 / sqrt(person2)* sqrt(person3);
}

//Ԥ���Ӱ�����ֺ���
double predictRating(Film test_film, int user_id, SimilarFilm *similar_film, int num)
{
	double top_sum = 0;
	double down_sum = 0;

	RatingMap::iterator ratingIt;
	RatingMap::iterator ratingEnd;
	for(int i = 0;i < num;i++)
	{
		ratingIt = similar_film[i].film.rating_map.find(user_id);
		ratingEnd = similar_film[i].film.rating_map.end();
		if(ratingIt != ratingEnd)
		{
			top_sum += similar_film[i].similarity * (ratingIt->second - similar_film[i].film.average_rating);
		}
		down_sum += fabs(similar_film[i].similarity);
		similar_film[i].similarity = 0;
	}

	if(down_sum < 0.00001)
		return test_film.average_rating;
	return int((test_film.average_rating + top_sum /down_sum)+0.5);
}
