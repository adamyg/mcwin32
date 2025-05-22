
# volinfo - volume information

A test framework which simulates the existing drive mapping emulation logic, analyses access times, and reports the corresponding delays.

Facilitates the diagnosis and testing of the mounted system file interface, illuminating any access issues/bottle necks.

## Usage

```
volinfo [options] [attributes] <operation>

   Query disk information using on the following operations:

Operations:
   volumes -           Iterate by volume enumeration.
   network -           Iterate by network enumeration.
   drives -            Iterate published drive letters.
   all -               All available methods.

Attributes:
   --volumeinfo        Volume information.
   --drivetype         Drive type.
   --attributes        Attributes.
   --freespace         Free space.
   --statfs            statfs, all attributes.

Options:
   --time              Access timestamps.
   --verbose           Additional info.
   --netconnected      Network status (default: connected).
   or --netremembered.
   --help
```
