#include "stdlib.h"

void* cn_malloc(unsigned long size);
void cn_free_sized(void*, size_t len);

struct sllist {
  int head;
  struct sllist* tail;
};

/*@
datatype List {
  Nil {},
  Cons {i32 Head, datatype List Tail}
}
@*/
/*@
function (i32) Hd (datatype List L) {
  match L {
    Nil {} => {
      0i32
    }
    Cons {Head : H, Tail : _} => {
      H
    }
  }
}

function (datatype List) Tl (datatype List L) {
  match L {
    Nil {} => {
      Nil{}
    }
    Cons {Head : _, Tail : T} => {
      T
    }
  }
}
@*/

struct dllist {
  int data;
  struct dllist* prev;
  struct dllist* next;
};

/*@
datatype Dll {
    Empty_Dll    {},
    Nonempty_Dll {datatype List left,
                  struct dllist curr,
                  datatype List right}
}
@*/
/*@
function (datatype List) Right_Sublist (datatype Dll L) {
    match L {
        Empty_Dll {} => { Nil{} }
        Nonempty_Dll {left: _, curr: _, right: r} => { r }
    }
}

function (datatype List) Left_Sublist (datatype Dll L) {
    match L {
        Empty_Dll {} => { Nil {} }
        Nonempty_Dll {left: l, curr: _, right: _} => { l }
    }
}

function (struct dllist) Node (datatype Dll L) {
    match L {
        Empty_Dll {} => {  struct dllist { data: 0i32, prev: NULL, next: NULL } }
        Nonempty_Dll {left: _, curr: n, right: _} => { n }
    }
}

predicate (datatype Dll) Dll_at (pointer p) {
    if (is_null(p)) {
        return Empty_Dll{};
    } else {
        take n = Owned<struct dllist>(p);
        take L = Own_Backwards(n.prev, p, n);
        take R = Own_Forwards(n.next, p, n);
        return Nonempty_Dll{left: L, curr: n, right: R};
    }
}

predicate (datatype List) Own_Forwards (pointer p,
                                        pointer prev_pointer,
                                        struct dllist prev_dllist) {
    if (is_null(p)) {
        return Nil{};
    } else {
        take P = Owned<struct dllist>(p);
        assert (ptr_eq(P.prev, prev_pointer));
        assert(ptr_eq(prev_dllist.next, p));
        take T = Own_Forwards(P.next, p, P);
        return Cons{Head: P.data, Tail: T};
    }
}

predicate (datatype List) Own_Backwards (pointer p,
                                         pointer next_pointer,
                                         struct dllist next_dllist) {
    if (is_null(p)) {
        return Nil{};
    } else {
        take P = Owned<struct dllist>(p);
        assert (ptr_eq(P.next, next_pointer));
        assert(ptr_eq(next_dllist.prev, p));
        take T = Own_Backwards(P.prev, p, P);
        return Cons{Head: P.data, Tail: T};
    }
}
@*/

struct dllist* singleton(int element)
  /*@ ensures take Ret = Dll_at(return);
          Ret == Nonempty_Dll {
                   left: Nil{},
                   curr: struct dllist {prev: NULL,
                                        data: element,
                                        next: NULL},
                   right: Nil{}};
  @*/
{
  struct dllist* n = cn_malloc(sizeof(struct dllist));
  n->data = element;
  n->prev = 0;
  n->next = 0;
  return n;
}
// Adds after the given node and returns a pointer to the new node
struct dllist* dll_insert_at(int element, struct dllist* n)
  /*@ requires take Before = Dll_at(n);
      ensures  take After = Dll_at(return);
               is_null(n) ?
                  After == Nonempty_Dll {
                             left: Nil{},
                             curr: Node(After),
                             right: Nil{}}
                : After == Nonempty_Dll {
                             left: Cons {Head: Node(Before).data,
                                         Tail: Left_Sublist(Before)},
                             curr: Node(After),
                             right: Right_Sublist(Before)};
  @*/
{
  struct dllist* new_dllist = cn_malloc(sizeof(struct dllist));
  new_dllist->data = element;

  if (n == 0) //empty list case
  {
    new_dllist->prev = 0;
    new_dllist->next = 0;
    return new_dllist;
  }
  else {
    /*@ split_case(is_null(n->prev)); @*/
    //! //
    new_dllist->next = n->next;
    new_dllist->prev = n;
    //!! next_is_self //
    //! new_dllist->next = new_dllist; new_dllist->prev = new_dllist; //

    //! //
    if (n->next != 0) {
    //!! forget_to_check_for_next_null //
    //! if (1) { //

      /*@ split_case(is_null(n->next->next)); @*/
      //! //
      n->next->prev = new_dllist;
      //!! forget_to_set_next_prev //
      //! //
    }
    n->next = new_dllist;
    return new_dllist;
  }
}

struct dllist_and_int {
  struct dllist* dllist;
  int data;
};

// Remove the given node from the list and returns another pointer 
// to somewhere in the list, or a null pointer if the list is empty
struct dllist_and_int* dll_remove(struct dllist* n)
  /*@ requires !is_null(n);
               take Before = Dll_at(n);
               let Del = Node(Before);
      ensures  take Ret = Owned<struct dllist_and_int>(return);
               take After = Dll_at(Ret.dllist);
               Ret.data == Del.data;
               (is_null(Del.prev) && is_null(Del.next))
                 ? After == Empty_Dll{}
                 : (!is_null(Del.next) ?
                      After == Nonempty_Dll {left: Left_Sublist(Before),
                                             curr: Node(After),
                                             right: Tl(Right_Sublist(Before))}
                     : After == Nonempty_Dll {left: Tl(Left_Sublist(Before)),
                                              curr: Node(After),
                                              right: Right_Sublist(Before)});
  @*/
{
  struct dllist* temp = 0;
  //! //
  if (n->prev != 0) {
  //!! forget_to_check_prev_null //
  //! if (1) { //
    /*@ split_case(is_null(n->prev->prev)); @*/

    //! //
    n->prev->next = n->next;
    //!! confuse_prev_next_with_prev //
    //! n->prev = n->next; //

    //! //
    temp = n->prev;
    //!! forget_to_set_temp //
    //! //
  }
  if (n->next != 0) {
    /*@ split_case(is_null(n->next->next)); @*/
    n->next->prev = n->prev;

    //! //
    temp = n->next;
    //!! confuse_next_and_prev //
    //! temp = n->prev; //
  }
  struct dllist_and_int* pair = cn_malloc(sizeof(struct dllist_and_int));
  pair->dllist = temp;
  pair->data = n->data;
  cn_free_sized(n, sizeof(struct dllist));
  return pair;
}

// /*@
// function [rec] (datatype List) SLAppend(datatype List L1, datatype List L2) {
//   match L1 {
//     Nil {} => {
//       L2
//     }
//     Cons {Head : H, Tail : T}  => {
//       Cons {Head: H, Tail: SLAppend(T, L2)}
//     }
//   }
// }

// function [rec] (datatype Dll) DLAppend(struct dllist curr, datatype Dll xs, datatype Dll ys) {
//   match xs {
//     Empty_Dll {} => {
//       ys
//     }
//     Nonempty_Dll { left: left1, curr: _, right: right1 } => {
//       match ys {
//         Empty_Dll {} => {
//           xs
//         }
//         Nonempty_Dll { left: left2, curr: curr2, right: right2 } => {
//           Nonempty_Dll {
//             left: left1,
//             curr: curr,
//             right: SLAppend(
//               right1,
//               SLAppend(
//                 left2,
//                 Cons { Head: curr2.data, Tail: right2 }
//               )
//             )
//           }
//         }
//       }
//     }
//   }
// }
// @*/

// struct dllist* dll_append(struct dllist* xs, struct dllist* ys)
// /*@
//   requires
//     take Xs = Dll_at(xs);
//     take Ys = Dll_at(ys);
//   ensures
//     take Zs = Dll_at(return);
//     Zs == DLAppend(Node(Zs), Xs, Ys);
// @*/
// {
//   if (!xs) {
//     return ys;
//   }
//   else {
//     struct dllist* tmp = dll_append(xs->next, ys);
//     xs->next = tmp;
//     if (tmp) {
//       tmp->prev = xs;
//     }
//     return xs;
//   }
// }
