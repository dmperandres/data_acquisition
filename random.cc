//LIC

#include "random.h"

using namespace std;

//HEA

_random_normal::_random_normal()
{
  random_device Random_device;  //Will be used to obtain a seed for the random number engine
  Seed=Random_device();
  Generator.seed(Seed);
}

//HEA

_random_normal::_random_normal(double Mean, double Stddev, double Min1, double Max1):
      Distribution(Mean, Stddev)
{
  random_device Random_device;  //Will be used to obtain a seed for the random number engine
  Seed=Random_device();
  Generator.seed(Seed);

  if (Min1<=Max1){
    Min=Min1;
    Max=Max1;
  }
  else std::cout << "Error:_random_normal: Min > Max " << std::endl;
}

//HEA

void _random_normal::init(double Mean, double Stddev, double Min1, double Max1)
{
  if (Min1<=Max1){
    Distribution.param(std::normal_distribution<double>::param_type(Mean,Stddev));
    Min=Min1;
    Max=Max1;
  }
  else std::cout << "Error:_random_normal: Min > Max " << std::endl;
}

//HEA

void	_random_normal::seed()
{
  random_device Random_device;  //Will be used to obtain a seed for the random number engine
  Seed=Random_device();
  Generator.seed(Seed);
}

//HEA

void	_random_normal::set_seed(long int Seed1)
{
  Seed=Seed1;
  Generator.seed(Seed);
}

//HEA

long int _random_normal::get_seed()
{
  return Seed;
}

//HEA

void	_random_normal::save_seed()
{
  Saved_seed=Seed;
}

//HEA

void  _random_normal::load_seed()
{
  Seed=Saved_seed;
  Generator.seed(Seed);
  Distribution.reset();
}

//HEA

double _random_normal::value()
{
  do{
    Value=Distribution(Generator);
  } while (Value<Min || Value>Max);
  return Value;
}

//HEA

_random_uniform_int::_random_uniform_int()
{
  random_device Random_device;  //Will be used to obtain a seed for the random number engine
  Seed=Random_device();
  Generator.seed(Seed);
}

//HEA

_random_uniform_int::_random_uniform_int(int Min1,int Max1)
{
  random_device Random_device;  //Will be used to obtain a seed for the random number engine
  Seed=Random_device();
  Generator.seed(Seed);

  if (Min1<=Max1){
    Distribution.param(std::uniform_int_distribution<int>::param_type(Min1,Max1));
  }
  else std::cout << "Error:_random_uniform_int: Min > Max " << std::endl;
}

//HEA

void _random_uniform_int::init(int Min1,int Max1)
{
  if (Min1<=Max1){
    Distribution.param(std::uniform_int_distribution<int>::param_type(Min1,Max1));
  }
  else std::cout << "Error:_random_uniform_int: Min > Max " << std::endl;
}

//HEA

void _random_uniform_int::seed()
{
  random_device Random_device;  //Will be used to obtain a seed for the random number engine
  Seed=Random_device();
  Generator.seed(Seed);
}

//HEA

void _random_uniform_int::set_seed(long int Seed1)
{
  Seed=Seed1;
  Generator.seed(Seed);
}

//HEA

long int _random_uniform_int::get_seed()
{
  return Seed;
}

//HEA

void _random_uniform_int::save_seed()
{
  Saved_seed=Seed;
}

//HEA

void _random_uniform_int::load_seed()
{
  Seed=Saved_seed;
  Generator.seed(Seed);
  Distribution.reset();
}

//HEA

int _random_uniform_int::value()
{
  return Distribution(Generator);
}

//HEA

_random_uniform_double::_random_uniform_double()
{
  random_device Random_device;  //Will be used to obtain a seed for the random number engine
  Seed=Random_device();
  Generator.seed(Seed);
}

//HEA

_random_uniform_double::_random_uniform_double(double Min1,double Max1)
{
  random_device Random_device;  //Will be used to obtain a seed for the random number engine
  Seed=Random_device();
  Generator.seed(Seed);
  if (Min1<=Max1){
    Distribution.param(std::uniform_real_distribution<double>::param_type(Min1,Max1));
  }
  else std::cout << "Error:_random_uniform_double: Min > Max " << std::endl;
}

//HEA

void _random_uniform_double::init(double Min1,double Max1)
{
  if (Min1<=Max1){
    Distribution.param(std::uniform_real_distribution<double>::param_type(Min1,Max1));
  }
  else std::cout << "Error:_random_uniform_double: Min > Max " << std::endl;
}

//HEA

void _random_uniform_double::seed()
{
  random_device Random_device;  //Will be used to obtain a seed for the random number engine
  Seed=Random_device();
  Generator.seed(Seed);
}

//HEA

void _random_uniform_double::set_seed(long int Seed1)
{
  Seed=Seed1;
  Generator.seed(Seed);
}

//HEA

long int _random_uniform_double::get_seed()
{
  return Seed;
}

//HEA

void _random_uniform_double::save_seed()
{
  Saved_seed=Seed;
}

//HEA

void _random_uniform_double::load_seed()
{
  Seed=Saved_seed;
  Generator.seed(Seed);
  Generator.discard(700000);
  Distribution.reset();
}

//HEA

double _random_uniform_double::value()
{
  return Distribution(Generator);
}
