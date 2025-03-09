#include "./helpers.h"
#include <plonk/resource_id.h>

describe(resource_id, {
	it("should create a new integer ID", {
		ResourceId id0 = ResourceId::next();
		ResourceId id1 = ResourceId::next();
		ResourceId id2 = ResourceId::next();

		assert(id0 != id1);
		assert(id0 != id2);
		assert(id1 != id2);
	});

	it("should compare IDs", {
		ResourceId int_id0(123);
		ResourceId int_id1(123);
		ResourceId int_id2(321);
		ResourceId str_id0("foobar");
		ResourceId str_id1("foobar");
		ResourceId str_id2("banana");
		ResourceId nil_id0;
		ResourceId nil_id1;

		assert(int_id0 == int_id1)
		assert(int_id0 != int_id2)
		assert(str_id0 == str_id1)
		assert(str_id0 != str_id2)
		assert(nil_id0 == nil_id1)
		assert(int_id0 != str_id0)
		assert(int_id0 != nil_id0)
		assert(str_id0 != int_id0)
		assert(str_id0 != nil_id0)
	})
});
