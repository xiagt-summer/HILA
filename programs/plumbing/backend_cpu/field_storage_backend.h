#ifndef VANILLA_BACKEND
#define VANILLA_BACKEND

#include "../field_storage.h"


#ifndef layout_SOA
// Array of Structures layout (standard)

template<typename T> 
inline auto field_storage<T>::get(const int i, const int field_alloc_size) const 
{
      T value = ((T*)fieldbuf)[i];
      return value;
}

template<typename T>
template<typename A>
inline void field_storage<T>::set(const A &value, const int i, const int field_alloc_size)
{
  ((T*)fieldbuf)[i] = value;
}

template<typename T>
void field_storage<T>::allocate_field(lattice_struct * lattice) {
  fieldbuf = malloc( sizeof(T) * lattice->field_alloc_size() );
  if (fieldbuf == nullptr) {
    std::cout << "Failure in field memory allocation\n";
    exit(1);
  }
  #pragma acc enter data create(fieldbuf)
}

template<typename T>
void field_storage<T>::free_field() {
  #pragma acc exit data delete(fieldbuf)
  if (fieldbuf != nullptr)
    free(fieldbuf);
  fieldbuf = nullptr;
}



#else
// Now structure of arrays


template<typename T> 
inline auto field_storage<T>::get(const int i, const int field_alloc_size) const {
  assert( i < field_alloc_size);
  T value;
  real_t *value_f = (real_t *)(&value);
  for (int e=0; e<(sizeof(T)/sizeof(real_t)); e++) {
     value_f[e] = ((real_t*)fieldbuf)[e*field_alloc_size + i];
   }
  return value; 
}

template<typename T>
template<typename A>
inline void field_storage<T>::set(const A &value, const int i, const int field_alloc_size){
  assert( i < field_alloc_size);
  real_t *value_f = (real_t *)(&value);
  for (int e=0; e<(sizeof(T)/sizeof(real_t)); e++) {
    ((real_t*)fieldbuf)[e*field_alloc_size + i] = value_f[e];
  }
}

template<typename T>
void field_storage<T>::allocate_field(lattice_struct * lattice) {
  constexpr static int t_elements = sizeof(T) / sizeof(real_t);
  fieldbuf = malloc( t_elements*sizeof(real_t) * lattice->field_alloc_size() );
  if (fieldbuf == nullptr) {
    std::cout << "Failure in field memory allocation\n";
    exit(1);
  }
  #pragma acc enter data create(fieldbuf)
}

template<typename T>
void field_storage<T>::free_field() {
  #pragma acc exit data delete(fieldbuf)
  if (fieldbuf != nullptr)
    free(fieldbuf);
  fieldbuf = nullptr;
}

#endif //layout




template<typename T>
void field_storage<T>::gather_elements(char * buffer, std::vector<unsigned> index_list, lattice_struct * lattice) const {
  for (int j=0; j<index_list.size(); j++) {
    int index = index_list[j];
    T element = get(index, lattice->field_alloc_size());
    std::memcpy( buffer + j*sizeof(T), (char *) (&element), sizeof(T) );
  }
}


template<typename T>
void field_storage<T>::place_elements(char * buffer, std::vector<unsigned> index_list, lattice_struct * lattice) {
  for (int j=0; j<index_list.size(); j++) {
    T element = *((T*) ( buffer + j*sizeof(T) ));
    set(element, index_list[j], lattice->field_alloc_size());
  }
}

template<typename T>
void field_storage<T>::set_local_boundary_elements(direction dir, parity par, lattice_struct * lattice){}


#endif