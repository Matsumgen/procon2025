#ifndef FS_FIELD_DB_HPP_
#define FS_FIELD_DB_HPP_

#include <field.hpp>
#include <vector>
#include <memory>
#include <array>

namespace fsdb {
  struct Routes;
  struct SRoutes;
  typedef std::shared_ptr<Routes> Routes_ptr;
  typedef std::shared_ptr<SRoutes> SRoutes_ptr;
  struct Routes {
    Ope ope;            // root nodeはope.n = 0
    std::uint8_t size;  // next以下のRoutesの総数 = 経路の総数
    std::vector<Routes_ptr> next;

    Routes();
    Routes(Ope ope);
    Routes(std::vector<Routes_ptr> next);
    Routes(Ope ope, std::vector<Routes_ptr> next);

    std::uint8_t getSize();
    std::vector<Ope> getOperation(size_t idx);
    std::vector<std::vector<Ope>> getAllOperation();
  };

  struct FsField {
    std::vector<std::uint16_t> field;
    std::vector<std::array<std::uint16_t, 2>> pairs;
    std::uint8_t size;

    FsField() = default;
    FsField(vector<std::uint16_t> field, std::uint8_t size);
    FsField(const Field& f);
    std::uint16_t get(std::uint8_t x, std::uint8_t y) const;
    std::uint16_t get(std::array<std::uint8_t, 2> p) const;
    std::uint16_t getPair(std::uint8_t x, std::uint8_t y) const;
    std::uint16_t getPair(std::uint16_t x) const;
    bool inField(const std::uint8_t x, const std::uint8_t y) const;
    void rotate(Ope ope);
    void print() const;
  };

  struct SRoutes {
    Ope ope;
    FsField f;
    std::vector<SRoutes_ptr> next;
    std::array<std::uint8_t, 2> tg; // 揃える左上の位置
    
    bool isNext(std::array<std::uint8_t, 2> p1, std::array<std::uint8_t, 2> p2, std::array<std::uint8_t, 2> p3, std::array<std::uint8_t, 2> p4) const;
    void toNext(SRoutes_ptr& r, Ope ope) const;
    bool inOpe(const Ope ope) const;
    bool inOpe(const std::array<std::uint8_t, 3> ope) const;
    bool inField(const Ope ope) const;
    bool inField(const std::array<std::uint8_t, 3> ope) const;
    bool isEnd() const;
    bool check();
    
    Routes_ptr toRoutes();

  };

  template <typename... Args>
  Routes_ptr make_Routes_ptr(Args&&... args) {
    return std::make_shared<Routes>(std::forward<Args>(args)...);
  }

  template <typename... Args>
  SRoutes_ptr make_SRoutes_ptr(Args&&... args) {
    return std::make_shared<SRoutes>(std::forward<Args>(args)...);
  }

}


#endif
