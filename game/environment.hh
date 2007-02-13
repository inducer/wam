// ----------------------------------------------------------------------------
//  Description      : Environment management
// ----------------------------------------------------------------------------
//  Remarks          : none.
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_ENVIRONMENT
#define WAM_ENVIRONMENT




#include <ixlib_random.hh>
#include "utility/basic_types.hh"
#include "engine/gameject.hh"
#include "engine/tick.hh"
#include "engine/collision.hh"
#include "engine/drawable.hh"
#include "engine/persistence.hh"
#include "engine/network.hh"
#include "engine/message.hh"
#include "engine/resource.hh"
#include "game_names.hh"
#include "physics.hh"




// wEnvironmentManager -------------------------------------------------------
class wEnvironmentManager : public wTickReceiver, public wDrawable, public wShaped,
      public wPersistentGameject, public wNetworkedGameject,
      public wMessageReceiver
{
    typedef TUnsigned8 TMessageKind;
    static const TMessageKind NET_MKIND_DUMP = 0;
    static const TMessageKind NET_MKIND_MODIFICATION = 1;

    float Wind;
    float Gravity;
    float_random RandomNumGen;
    string BackgroundBitmap;

    wImage Foreground;
    wShapeBitmap Shape, HoleMap;
    wImageHolder GroundTile,HoleTile,Floor,Ceiling,HoleFloor,
      HoleCeiling;

    static const TUnsigned32 END = 0;
    static const TUnsigned32 THEME = 1;
    static const TUnsigned32 GADGET = 2;
    static const TUnsigned32 BRIDGE = 3;
    static const TUnsigned32 LOADED = 4;
    static const TUnsigned32 BUILT = 5;
    static const TUnsigned32 SHAPE = 6;
    static const TUnsigned32 HOLEMAP = 7;

    wStreamStore LandscapeBuildInstructions;

    /* consisting of either:

    [u32] LOADED
    [str] load_filename

    or:

    [u32] BUILT
    (
      [u32] THEME
      [str] theme_what (as in XML desc)
      [str] theme_value
    )*
    [u32] SHAPE
    shape
    [u32] HOLEMAP
    holemap
    (
      [u32] BRIDGE
      [s32] y
      [s32] left_end
      [s32] right_end
    )*
    (
      [u32] GADGET
      [str] image_name
      [s32] x
      [s32] y
    )*
    [u32] END

    */

    TMessageId	IdExplosion;

  public:
    wEnvironmentManager( wGame &s );
    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_ENVIRONMENTMGR;
    }

    void registerMe();
    void startUp();
    void prepareToDie();
    void unregisterMe();

    int getDrawPriority() const
    {
      return WAM_DRAWPRIO_LANDSCAPE;
    }
    wDisplayExtent getDisplayExtent();
    void draw( drawable &dpy );

    void tick( float millisec );

    ref<wMessageReturn>
    receiveMessage( TMessageId id, string const &par1, void *par2 );

    void loadLandscape( string const &name );
    void generateLandscape( string const &description, int seed = 0 );

    float getWind()
    {
      return Wind;
    }
    float getGravity()
    {
      return Gravity;
    }
    void setWind( float wind )
    {
      Wind = wind;
    }
    void setGravity( float gravity )
    {
      Gravity = gravity;
    }

    void applyGravity( wPhysicsVector &speed, float seconds );
    void applyWind( wPhysicsVector &speed, float seconds, float friction = 1 );

    // persistence ------------------------------------------------------------
    TPersistenceMode getPersistenceMode() const
    {
      return PERSISTENCE_SINGLETON;
    }
    TPriority getSaveOrder() const
    {
      return GJSAVE_MANAGER;
    }
    void load( xml_file::tag &tag );
    void save( xml_file::tag &tag );

    // network ----------------------------------------------------------------
    void notifyNewConnection( wNetworkConnection *cnx );
    void notifyEndedConnection( wNetworkConnection *cnx );
    void notifyRemoteInitiation( wNetworkConnection *cnx, wObject::TId remote_id );

    void readMessage( wNetworkConnection *from, wStreamReader &reader );
    void sendDump( wNetworkConnection *to, wNetworkConnection *except );

  private:
    static double deviate( float_random &rng, double value, double max_stepsize, double min, double max );
    static void generateLandscapePolygon( float_random &rng, string const &type,
                                          polygon<int> &poly, int x1, int y1, int x2, int y2 );
    static void digHoles( float_random &rng, wShapeBitmap &holemap, int x1, int y1, int x2, int y2 );
    static void messWithConvexPolygonSegment( float_random &rng, polygon_segment<int> &poly,
        int x1, int y1, int x2, int y2, double intensity = 0.25 );

    void generateLandscapeBuildInstructions( wStreamWriter &writer, string const &description, int seed = 0 );
    void executeLandscapeBuildInstructions( wStreamReader &reader );
    void executeLandscapeBuildInstructions_BUILT( wStreamReader &reader );
    void paintForeground( wImage &fg, int x1, int y1, int x2, int y2 );
    void punchHole( int x, int y, int radius );

    void updateManagers();
};




// registration ---------------------------------------------------------------
void registerEnvironmentManager( wGame &g );




#endif
