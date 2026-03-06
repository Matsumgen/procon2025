#include <algo2lib.hpp>
#include <algo2.cuh>
namespace algo2lib {

bool tail_counter::fetch_tail(uint32_t& t, uint32_t& sc) {
  std::lock_guard lock(this->mtx);
  sc =  this->tail % MemObj2::QUEUE_SIZE;
  if(this->q->done[sc]) {
    t = this->tail++;
    return true;
  }
  t = this->tail;
  return false;
}

bool tail_counter::get(uint16_t *scores, uint32_t *rid, uint32_t *fid) {
  uint32_t t, sc;
  if(!this->fetch_tail(t, sc)){
    if(this->field_len <= ((uint32_t)(t / this->maxRid)) * MemObj2::FIELDS_PER_THREAD) *fid = 0xffffffff;
    return false;
  }
  *fid = ((uint32_t)(t / this->maxRid)) * MemObj2::FIELDS_PER_THREAD;
  if(this->field_len <= *fid) {
    *fid = 0xffffffff;
    return false;
  }

  for(size_t i = 0; i < (MemObj2::FIELDS_PER_THREAD >> 1); ++i) {
    uint32_t score = this->q->scores[(MemObj2::FIELDS_PER_THREAD >> 1) * sc + i];
    scores[2*i] = score >> 16;
    scores[2*i + 1] = score & 0xffff;
  }
  this->q->done[sc] = 0;
  this->emp_num.fetch_add(1);
  *rid = t % this->maxRid;
  if(*rid == 0) { this->cv.notify_all(); }
  return true;
}

void tail_counter::addTask(uint32_t tasks) {
  std::lock_guard lock(this->mtx);
  this->emp_num.fetch_sub(tasks);
}

void tail_counter::wait(uint32_t need_emp) {
  /* if(this->emp_num < need_emp) printf("[run gpu] wait\n"); */
  std::unique_lock<std::mutex> lock(this->emp_num_mtx);
  cv.wait(lock, [&] { return this->emp_num.load() >= need_emp; });
  /* printf("wait end need_emp=%d, emp_num=%d\n", need_emp, this->emp_num); */
}

void tail_counter::clear() {
  this->tail = 0;
  this->emp_num.store(MemObj2::QUEUE_SIZE);
}

// debug
void rotateField(RawField& field, const uint32_t fsize, const Ope ope){
  rotateField(field, fsize, ope.x(), ope.y(), ope.n());
}
void rotateField(RawField& field, const uint32_t fsize, const uint32_t x, const uint32_t y, const uint32_t n) {
  uint32_t n_half = n >> 1;

  uint32_t h, w;
  uint32_t i0, i1, i2, i3, buf;
  for(h = 0; h < n_half; ++h) {
    i0 = x + (y+h) * fsize;
    i1 = x + (n - 1 - h) + y * fsize;
    i2 = (x + (n - 1)) + (y + (n - 1 - h)) * fsize;
    i3 = (x + h) + (y + (n - 1)) * fsize;
    for(w = 0; w < n_half; ++w){
      buf = field[i0];
      field[i0] = field[i3];
      field[i3] = field[i2];
      field[i2] = field[i1];
      field[i1] = buf;

      ++i0;
      i1 += fsize;
      --i2;
      i3 -= fsize;
    }
  }

  if(n & 1){
    i0 = x+n_half + y * fsize;
    i1 = x + n - 1 + (y + n_half) * fsize;
    i2 = x+n_half + (y + n - 1) * fsize;
    i3 = x + (y + n_half) * fsize;
    for(h = 0; h < n_half; ++h) {
      buf = field[i0];
      field[i0] = field[i3];
      field[i3] = field[i2];
      field[i2] = field[i1];
      field[i1] = buf;
      i0 += fsize;
      --i1;
      i2 -= fsize;
      ++i3;
    }
  }

}


}
