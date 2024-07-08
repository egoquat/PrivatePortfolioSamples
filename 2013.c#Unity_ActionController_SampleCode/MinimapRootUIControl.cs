#define EGOQUAT_SAMPLE_WORLDFIND_121122

using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MinimapRootUIControl : MonoBehaviour {
    protected class MinimapAtlasSet
    {
        public Transform gameUnit;
        public UISprite atlas;
        public bool isStaticAtlas;
        public MinimapAtlasSet(Transform gameUnit, UISprite atlas, bool isStaticAtlas)
        {
            this.gameUnit = gameUnit;
            this.atlas = atlas;
            this.isStaticAtlas = isStaticAtlas;
        }
    }

    public string nameOfWorldObject = "test01";
    public Transform rootWorldmap = null;
    public UIPanel panelRootMinimap = null;
    public Transform tmMinimapMin = null;
    public Transform tmMinimapMax = null;

    public UISprite heroAtlas = null;
    public UISprite allyAtlas = null;
    public UISprite enermyAtlas = null;
    public UISprite goalAtlas = null;
    public UISprite towerActiveAtlas = null;
    public UISprite towerUnActiveAtlas = null;
    public UISprite npcUnAggressiveAtlas = null;
    public UISprite npcAggressiveAtlas = null;

    protected bool minimapSynchWithWorld = false;
    protected bool saveToWorldShotTexture = false;
    protected bool synchedWithWorld = false;
    protected bool processingMinimap = false;

    protected Vector3 posMinWorld = Vector3.zero;
    protected Vector3 posMaxWorld = Vector3.zero;
    protected Vector3 posCenterOfWorld = Vector3.zero;

    protected Vector3 posMinMinimap = Vector3.zero;
    protected Vector3 posMaxMinimap = Vector3.zero;
    protected Vector3 posCenterMinimap = Vector3.zero;

    protected Vector2 ratioTransforming = Vector2.one;

    //@ ScreenShot
    protected float nearScreenShot = -10.0f;
    protected float farScreenShot = 1000.0f;
    protected Color colorAmbientScreenShot = new Color(0.6f, 0.6f, 0.6f, 1.0f);
    protected Color colorMinimapTexture = new Color(1.0f, 1.0f, 1.0f, 0.8f);
    protected Vector3 offsetScreenShot = new Vector3(0.0f, 0.0f, -100.0f);
    protected Vector3 offsetPosMinimapMaterial = new Vector3(0.0f, 0.0f, -20.0f);

    protected int widthMinimapBuffer = 0;
    protected int heightMinimapBuffer = 0;
    
    //@test //미니맵 변환을 위한 월드 정점들 제대로 가져오는지 조사 용도.
    //List<Vector3> listVertices = new List<Vector3>();
    Vector3 scaleDrawGizmo = new Vector3(4.0f, 4.0f, 4.0f);
    MinimapAtlasMovementUIControl minimapMovementController = new MinimapAtlasMovementUIControl();
    List<MinimapAtlasSet> preOrderedAtlases = new List<MinimapAtlasSet>();

    public void SetEnableAtlas(Transform gameUnit)
    {
        minimapMovementController.SetEnableAtlas(gameUnit, true);
    }

    public void SetDisableAtlas(Transform gameUnit)
    {
        minimapMovementController.SetEnableAtlas(gameUnit, false);
    }

    public void SetDeleteAtlas(Transform gameUnit)
    {
        minimapMovementController.DeleteAtlasInstancing(gameUnit);
    }

    void AddHeroAtlas(Transform gameUnit)
    {
        AddNewAtlasInstancing(gameUnit, heroAtlas, false);
    }

    void AddAllyAtlas(Transform gameUnit)
    {
        AddNewAtlasInstancing(gameUnit, allyAtlas, false);
    }

    void AddEnermyAtlas(Transform gameUnit)
    {
        AddNewAtlasInstancing(gameUnit, enermyAtlas, false);
    }

    void AddGoalAtlas(Transform gameUnit)
    {
        AddNewAtlasInstancing(gameUnit, goalAtlas, true);
    }

    void AddTowerActiveAtlas(Transform gameUnit)
    {
        AddNewAtlasInstancing(gameUnit, towerActiveAtlas, true);
    }

    void AddTowerUnActiveAtlas(Transform gameUnit)
    {
        AddNewAtlasInstancing(gameUnit, towerUnActiveAtlas, true);
    }

    void AddNpcUnAggressiveAtlas(Transform gameUnit)
    {
        AddNewAtlasInstancing(gameUnit, npcUnAggressiveAtlas, false);
    }

    void AddNpcAggressiveAtlas(Transform gameUnit)
    {
        AddNewAtlasInstancing(gameUnit, npcAggressiveAtlas, false);
    }

    public void AddActorAtlasWithTeamSet(Transform gameUnit)
    {
        Identifier id = gameUnit.GetComponent<Identifier>();
        if (null == id) { Debug.LogError("(null == id)/" + Time.time); }
        TeamRelative teamside = id.GetTeamRelativeWithMine();

        if (true == id.IsObjectTypeNPCTower())
        {
            AddTowerActiveAtlas(gameUnit);
            return;
        }

        switch (teamside)
        {
            case TeamRelative.Mine:
                {
                    AddHeroAtlas(gameUnit);
                }
                break;
            case TeamRelative.Ally:
                {
                    AddAllyAtlas(gameUnit);
                }
                break;
            case TeamRelative.Enermy:
                {
                    AddEnermyAtlas(gameUnit);
                }
                break;
            case TeamRelative.None:
                {
                    AddNpcUnAggressiveAtlas(gameUnit);
                }
                break;
        }
    }

    void AddNewAtlasInstancing(Transform gameUnit, UISprite atlas, bool isStaticAtlas)
    {
        if (false == minimapMovementController.isInitialized) { preOrderedAtlases.Add(new MinimapAtlasSet(gameUnit, atlas, isStaticAtlas)); }
        else
        {
            minimapMovementController.AddNewAtlasInstancing(gameUnit, atlas, isStaticAtlas);
        }
    }

    string NamingTextureFile(int width, int height)
    {
        string pathDirectory = Application.dataPath + "/assets/Temp/ScreenShot/";
        if(false == System.IO.Directory.Exists(pathDirectory))
        { 
            System.IO.DirectoryInfo newPath = new System.IO.DirectoryInfo(pathDirectory);
            newPath.Create();
        }
        return string.Format("{0}screen_{1}x{2}_{3}.png",
                             pathDirectory,
                             width, height,
                             System.DateTime.Now.ToString("MM-dd_HH-mm-ss"));
    }

    void ScreenShotWorldToBuffer()
    {
        Vector2 minimapScreen = new Vector2(widthMinimapBuffer, heightMinimapBuffer);
        Vector2 worldScreen = new Vector2(posMaxWorld.x - posMinWorld.x, posMaxWorld.y - posMinWorld.y);
        ScreenShotWorldToBuffer(posCenterOfWorld, posMinMinimap + offsetPosMinimapMaterial, worldScreen, minimapScreen,
                                offsetScreenShot, nearScreenShot, farScreenShot );
    }

    void ScreenShotWorldToBuffer(   Vector3 positionCenterWorldGeometry, 
                                    Vector3 positionBatchOnMinimap,
                                    Vector2 worldScreen,
                                    Vector2 minimapScreen,
                                    Vector3 offsetScreenShotCamera,
                                    float nearScreenShotCamera,
                                    float farScreenShotCamera )
    {
        float widthWorld = worldScreen.x;
        float heightWorld = worldScreen.y;
        int widthOutTexture = (int)minimapScreen.x;
        int heightOutTexture = (int)minimapScreen.y;

        GameObject cameraObject = new GameObject("Camera MinimapScreenShot");
        Camera cameraOrthogonal = cameraObject.AddComponent<Camera>();
        cameraOrthogonal.transform.position = positionCenterWorldGeometry - offsetScreenShotCamera;
        cameraOrthogonal.transform.LookAt(positionCenterWorldGeometry);
        cameraOrthogonal.near = nearScreenShotCamera;
        cameraOrthogonal.far = farScreenShotCamera;
        cameraOrthogonal.orthographic = true;
       
        //@OrthogonalSize == height값이고, 결과 Texture Width에 비율을 맞춰준다.
        cameraOrthogonal.orthographicSize = widthWorld * ((float)Screen.height/(float)Screen.width) * 0.5f;

        float ratioMultiplier = widthWorld / (heightWorld * ((float)widthOutTexture / (float)heightOutTexture));
        cameraOrthogonal.orthographicSize = heightWorld * 0.5f * ratioMultiplier;

        #region region_screenshot_worldmap
        Color colorAmbientBackup = RenderSettings.ambientLight;
        RenderSettings.ambientLight = colorAmbientScreenShot;

        RenderTexture rt = new RenderTexture(widthOutTexture, heightOutTexture, 24); 
        cameraOrthogonal.targetTexture = rt;

        Texture2D textureScreenShot = new Texture2D(widthOutTexture, heightOutTexture, TextureFormat.RGB24, false);
        cameraOrthogonal.Render();
        RenderTexture.active = rt;
        textureScreenShot.ReadPixels(new Rect(0, 0, widthOutTexture, heightOutTexture), 0, 0);

        RenderSettings.ambientLight = colorAmbientBackup;
        #endregion

        cameraOrthogonal.targetTexture = null;
        RenderTexture.active = null; // JC: added to avoid errors
        Destroy(rt);

        byte[] buffer = textureScreenShot.EncodeToPNG();

        #region savetofile_minimap
#if UNITY_EDITOR
        if (true == saveToWorldShotTexture)
        {
            string filepath = NamingTextureFile(textureScreenShot.width, textureScreenShot.height);
            System.IO.File.WriteAllBytes(filepath, buffer);
            Debug.Log(string.Format("Saved World view for Minimap. Took texture to: {0} buffer{1}//{2}", filepath, buffer.Length, Time.time));  //@test
        }
#endif
        #endregion

        textureScreenShot = null;

        Texture2D textureNew = new Texture2D(widthOutTexture, heightOutTexture, TextureFormat.RGB24, false);
        textureNew.LoadImage(buffer);

        Material materialMinimap = new Material(Shader.Find("Transparent/Vertex Colored"));
        materialMinimap.SetColor("_Color", colorMinimapTexture);
        materialMinimap.mainTexture = textureNew;

        Destroy(cameraOrthogonal.gameObject);  //@ Destroy Screens Shot Camera

        GameObject minimapObject = new GameObject("Minimap Material");
        panelRootMinimap.transform.setChild(minimapObject.transform, positionBatchOnMinimap, Vector3.zero, Vector3.one);

        GeometryHelper.CreateSimplePlaneOnRenderer(minimapObject, materialMinimap, heightOutTexture, widthOutTexture, 1, 1);
    }

    void setHide(MonoBehaviour unit)
    {
        if (null != unit) { unit.gameObject.active = false; } else { return; }
    }

    private bool getAABBFromWorld(ref Vector3 posmin, ref Vector3 posmax)
    {
        GameObject[] worldmaps = GameObject.FindGameObjectsWithTag("Worldmap");
        GameObject[] worldmapsLayer = GameObjectHelper.FindGameObjectsWithLayer(LayerMask.NameToLayer("Worldmap"));
        List<GameObject> worldmapsFoundList = new List<GameObject>();
        worldmapsFoundList.AddRange(worldmaps); worldmapsFoundList.AddRange(worldmapsLayer);

        GameObject foundWorld = null;

        if (worldmapsFoundList.Count == 0) { Debug.Log("No Worldmap/" + Time.time); return false; }

        if (worldmapsFoundList.Count > 1)
        {
            foreach (GameObject g in worldmapsFoundList)
            {
                if (null != g.GetComponent<WorldmapGUISampleAttacher>()) { foundWorld = g; break; }
            }
            if (foundWorld == null)
            {
                foundWorld = worldmapsFoundList[0].transform.root.gameObject;
            }
        }
        else
        {
            foundWorld = worldmapsFoundList[0];
        }

        if (null == foundWorld)
        {
            Debug.Log("No Worldmap/" + Time.time);
            return false;
        }
        rootWorldmap = foundWorld.transform;

        GeometryHelper.GetPointsMinMaxRecursively(rootWorldmap, ref posmin, ref posmax);

        synchedWithWorld = true;

        ////testdebug
        //Debug.Log("AABB FromWorld("+(posmin)+"/"+(posmax)+")/" + Time.time);

        return true;
    }

    private bool getAABBFromMapmgr(ref Vector3 posmin, ref Vector3 posmax)
    {
        MapColliderInfo mapcolliderinfo = MapMgr.instance.currenMapColliderInfo;
        posmin = mapcolliderinfo.info.map.min;
        posmax = mapcolliderinfo.info.map.max;

        if (VectorHelper.isSimilar(posmin, Vector3.zero) && VectorHelper.isSimilar(posmax, Vector3.zero)) { return false; }
        
        ////testdebug
        //Debug.Log("AABB FromMapMgr(" + (posmin) + "/" + (posmax) + ")/" + Time.time);

        return true;
    }

	// Use this for initialization
	void Start () {
        if (null == panelRootMinimap) { Debug.LogWarning("(null == panelRootMinimap)"); }
        if (null == tmMinimapMin) { Debug.LogWarning("(null == tmMinimapMin)"); }
        if (null == tmMinimapMax) { Debug.LogWarning("(null == tmMinimapMax)"); }

        setHide(heroAtlas);
        setHide(allyAtlas);
        setHide(enermyAtlas);
        setHide(goalAtlas);
        setHide(towerActiveAtlas);
        setHide(towerUnActiveAtlas);
        setHide(npcUnAggressiveAtlas);
        setHide(npcAggressiveAtlas);

        posMinMinimap = tmMinimapMin.localPosition;
        posMaxMinimap = tmMinimapMax.localPosition;

        if (true == minimapSynchWithWorld || null == MapMgr.instance || null == MapMgr.instance.currenMapColliderInfo)
        {
            if (false == getAABBFromWorld(ref posMinWorld, ref posMaxWorld))
            { if (false == getAABBFromMapmgr(ref posMinWorld, ref posMaxWorld)) return; }
        }
        else
        {
            if (false == getAABBFromMapmgr(ref posMinWorld, ref posMaxWorld))
            { if (false == getAABBFromWorld(ref posMinWorld, ref posMaxWorld)) return; }
        }
        
        posCenterOfWorld = (posMinWorld +posMaxWorld) * 0.5f;
        widthMinimapBuffer = (int)(posMaxMinimap.x - posMinMinimap.x);
        heightMinimapBuffer = (int)(posMaxMinimap.y - posMinMinimap.y);
        ratioTransforming = new Vector3(widthMinimapBuffer / (posMaxWorld.x - posMinWorld.x), widthMinimapBuffer / (posMaxWorld.x - posMinWorld.x), 1.0f);
        Vector2 minimapRect = new Vector2(widthMinimapBuffer, heightMinimapBuffer);

        float offsetMinimapY = (((posMaxMinimap.y - posMinMinimap.y) * 0.5f)
                - ((posMaxMinimap.x - posMinMinimap.x) * ((posMaxWorld.y - posMinWorld.y) / (posMaxWorld.x - posMinWorld.x)) * 0.5f));

        ScreenShotWorldToBuffer();

        minimapMovementController.InitializeMinimapAtlas(panelRootMinimap, posMinWorld, posMinMinimap, minimapRect, ratioTransforming, offsetMinimapY);

        if (false == minimapMovementController.isInitialized) { Debug.LogError("(false == minimapMovementController.isInitialized)"); }

        foreach (MinimapAtlasSet atlasset in preOrderedAtlases)
        {
            AddNewAtlasInstancing(atlasset.gameUnit, atlasset.atlas, atlasset.isStaticAtlas);
        }
        preOrderedAtlases.Clear();

        processingMinimap = true;
	}

	void Update () 
    {
        if (false == processingMinimap) return;

        minimapMovementController.UpdateMinimapAtlas();
	}

    void LateUpdate()
    {
        if (false == processingMinimap) return;

        //if (false == tookshot)
        //{
        //    ScreenShotWorldToBuffer();
        //    tookshot = true;
        //}
    }

    //@Minimap Test Draw
    void OnDrawGizmos()
    {
        if (false == processingMinimap) return;

        Gizmos.color = Color.red;
        { 
            Gizmos.DrawCube(posMinWorld, scaleDrawGizmo);
            GizmosDrawText.Instance.DrawText(Camera.current, posMinWorld + new Vector3(0.0f, -5.0f, 0.0f), posMinWorld.ToString());
        }
        { 
            Gizmos.DrawCube(posMaxWorld, scaleDrawGizmo);
            GizmosDrawText.Instance.DrawText(Camera.current, posMaxWorld + new Vector3(0.0f, 5.0f, 0.0f), posMaxWorld.ToString());
        }
        Vector3 bound = posMaxWorld - posMinWorld;
        Gizmos.DrawWireCube((posMinWorld + posMaxWorld) * 0.5f, bound);
    }
}
