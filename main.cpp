// Oleg Kotov

#include "slotmap.h"
#include <string>

int main()
{
	SlotMap<std::string, 4> slotmap;

	Handle foo = slotmap.add( "foo" );
	Handle bar = slotmap.add( "bar" );

	assert( slotmap[foo] == "foo" );
	assert( slotmap[bar] == "bar" );

	slotmap.remove( bar );

	Handle reuse = slotmap.add( "reuse" );

	assert( slotmap.contains( bar ) == false );

	return 0;
}

