#include <Field.hpp>
#include <unordered_map>

FieldChild::FieldChild(Field *f, const int x, const int y, const int n)
: parent(f), answer_size(f->getAnswer().size()), px(x), py(y)
{
  this->size = n;
  this->field = new ENT**[n];
  this->confirm = new int*[n];
  ENT *ent;
  for(int dy = 0; dy < n; dy++){
    this->field[dy] = new ENT*[n];
    this->confirm[dy] = new int[n]{};
    for(int dx = 0; dx < n; dx++){
      this->field[dy][dx] = new ENT;
      *(this->field[dy][dx]) = *(f->get(x + dx, y + dy));
      this->confirm[dy][dx] = f->isConfirm(x + dx, y + dy);
    }
  }
  this->correspondence = Field::reallocation(*this);
  const int numSize = correspondence.size();
  this->pentities = new PENT[numSize];
  int nn;
  for(int dy = 0; dy < n; dy++){
    for(int dx = 0; dx < n; dx++){
      nn = this->field[dy][dx]->num;
      if(this->pentities[nn].p1 == nullptr){
        this->pentities[nn].p1 = this->field[dy][dx];
      }else{
        this->pentities[nn].p2 = this->field[dy][dx];
      }
    }
  }

}

// 親Fieldに自身を反映させる
void FieldChild::reflection(){
  this->parent->reflection(this, this->px, this->py, this->answer_size, this->correspondence);
}
