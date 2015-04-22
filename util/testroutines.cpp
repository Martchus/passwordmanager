#include "testroutines.h"

#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>

#include <iostream>
#include <string>
#include <sstream>

using namespace std;
using namespace IoUtilities;

namespace Testroutines {

void lengthPrefixedString()
{
    stringstream stream;
    BinaryReader reader(&stream);
    BinaryWriter writer(&stream);
    string string1("jöalfj32öl4fj34 f234ölf3je frasdölkajwe fqwöejkfwöfklja sdefölasje fasef jasöefjas efajs eflasje faöslefj asöflej asefölajsefl öasejföaslefja söef jaseö flajseflas jeföaslefj aslefjaweöflja4 rfq34jqlök4jfq ljase öfaje4fqp 34f89uj <pfj apefjawepfoi jaefoaje föasdjfaösefj a4jfase9fau sejfpas");
    string string2;
    string string3("asdfalsjd23öl4j3");
    writer.writeLengthPrefixedString(string1);
    writer.writeLengthPrefixedString(string2);
    writer.writeLengthPrefixedString(string3);
    if(reader.readLengthPrefixedString() == string1
            && reader.readLengthPrefixedString() == string2
            && reader.readLengthPrefixedString() == string3) {
        cout << "test sucessfull" << endl;
    } else {
        cout << "test failed" << endl;
    }
}

}


