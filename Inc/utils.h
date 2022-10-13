//
// Created by lab on 10/12/22.
//

#ifndef AUTOMAC_UTILS_H
#define AUTOMAC_UTILS_H

#define MAC_ADDRESS_LENGTH (6)

#define new(type) ((type*)malloc(sizeof (type)))
#define new_array(type, length) ((type*)malloc(sizeof(type) * length))

#define lambda(return_type, function_body) \
({ \
      return_type $this function_body \
      $this; \
})
#define $ lambda

#endif //AUTOMAC_UTILS_H
