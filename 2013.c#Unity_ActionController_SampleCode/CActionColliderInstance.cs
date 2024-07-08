using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;

[System.Serializable]
public class CActionColliderInstance
{
    public enum etypeactioncollider
    {
        collider_box,
        collider_capsule,
        collider_sphere,
        collider_cylinder,
        collider_box2D
    }


    [System.Serializable]
    public class CShapeActionCollider
    {
        public Mesh _meshcolliderBox;
        public Mesh _meshcolliderCylinder;
        public Mesh _meshcolliderCapsule;
        public Mesh _meshcolliderSphere;
        public Mesh _meshcolliderFrustum;
        

        public Material _materialCollider;

        private Dictionary<etypeactioncollider,Mesh> _shapes;
        public Dictionary<etypeactioncollider,Mesh> shapes
        {
            get
            {
                if (_shapes != null)
                    return _shapes;
                _shapes = new Dictionary<etypeactioncollider,Mesh>();
                _shapes.Add(etypeactioncollider.collider_box, _meshcolliderBox);
                _shapes.Add(etypeactioncollider.collider_cylinder, _meshcolliderCylinder);
                _shapes.Add(etypeactioncollider.collider_capsule, _meshcolliderCapsule);
                _shapes.Add(etypeactioncollider.collider_sphere, _meshcolliderSphere);

                return _shapes;
            }
        }
    }

    public enum eaxisneartofar
    {
        axisz = 0,
        axisx = 1,
        axisy = 2,
    }

    [System.Serializable]
    public class CNearFarParam
    {
        public eaxisneartofar _axisNeartoFar = eaxisneartofar.axisz;
        public Vector3 _scaleNear = new Vector3(1.0f, 1.0f, 1.0f);
        public Vector3 _scaleFar = new Vector3(1.0f, 1.0f, 1.0f);
        public bool _doNeartoFar = false;
    }

    class CVertexDistance
    {
        public float _dist;
        public Vector3 _pos;
        
        public void setVertex(float dist, Vector3 position)
        {
            _dist = dist;
            _pos = position;
        }
    }

    class crendercolliderfordebug
    {
        public Mesh meshCollider = null;
        public Material materialCollider = null;
    }

    public etypeactioncollider _typecollider = etypeactioncollider.collider_capsule;

    public Vector3 _sizeCollider = new Vector3(1.0f, 1.0f, 1.0f);
    public Vector3 _positionCollider = new Vector3(0.0f, 0.0f, 0.0f);
    public CActionColliderHandler ownerHandler;

    public enum edetachtiming
    {
        none,
        whenstart,
        whenactive
    }
    public edetachtiming _detachMovement = edetachtiming.none;

    public float _timeCollideDelay = 0.0f;
    public float _timeCollideDuration = 20.0f;

    public CNearFarParam _nearfarParam = new CNearFarParam();

    Collider _collider = null;
    bool _processColliderAction = false;
    bool _startedColliderActivation = false;
    bool _enabledColliderAction = false;
    float _timeStarted = -1.0f;
    CFixPositioning _attachablehierarchy = new CFixPositioning();
    public bool _shouldRenderDebugMeshes = true;

    public struct BOX2DInfo
    {
        public float LEFT;
        public float TOP;
        public float RIGHT;
        public float BOTTOM;
        public bool bUseBox2D;
        public bool collisionBoxPiercing;
        public BonePosition position;
    };

    public BOX2DInfo box2DInfo;

    crendercolliderfordebug _rendercolliderfordebug = null;
    Dictionary<etypeactioncollider, AttachColliderWhichtype> _colliderAttachers = new Dictionary<etypeactioncollider, AttachColliderWhichtype>();


    Mesh copymeshFromNearToFar(Mesh meshSrc, Vector3 scale, Vector3 translate, CNearFarParam nearfar)
    {
        if (!meshSrc)
            return null;

        Vector3[] verticesNew = new Vector3[meshSrc.vertices.Length];
        int[] trianglesNew = new int[meshSrc.triangles.Length];
        Vector2[] uvNew = new Vector2[meshSrc.uv.Length];
        Color[] colorNew = new Color[meshSrc.colors.Length];
        Vector4[] tangentsNew = new Vector4[meshSrc.tangents.Length];

        Array.Copy(meshSrc.triangles, trianglesNew, meshSrc.triangles.Length);
        Array.Copy(meshSrc.uv, uvNew, meshSrc.uv.Length);
        Array.Copy(meshSrc.colors, colorNew, meshSrc.colors.Length);
        Array.Copy(meshSrc.tangents, tangentsNew, meshSrc.tangents.Length);

        {
            if (true == nearfar._doNeartoFar)
            {
                CVertexDistance[] verticesDist = new CVertexDistance[meshSrc.vertices.Length];
                Vector3 dirAxis = (new Vector3[] { Vector3.forward, Vector3.right, Vector3.up })[(int)nearfar._axisNeartoFar];
                float distmin = float.MaxValue, distmax = float.MinValue, rangedist = 0.0f;
                for (int idxVer = 0; idxVer < meshSrc.vertices.Length; ++idxVer)
                {
                    CVertexDistance vertexdist = new CVertexDistance();
                    Vector3 vertexPos = Vector3.Scale(meshSrc.vertices[idxVer], scale);
                    vertexdist.setVertex(Vector3.Dot(dirAxis, vertexPos), vertexPos);
                    verticesDist[idxVer] = vertexdist;

                    if (distmax < vertexdist._dist) distmax = vertexdist._dist;
                    if (distmin > vertexdist._dist) distmin = vertexdist._dist;
                }

                rangedist = Mathf.Abs(distmax - distmin);

                for (int idxVer = 0; idxVer < verticesDist.Length; ++idxVer)
                {
                    CVertexDistance vertexdist = verticesDist[idxVer];
                    float ratioDist = (Mathf.Abs(vertexdist._dist - distmin) / rangedist);
                    Vector3 ratioScale = nearfar._scaleNear + ((nearfar._scaleFar - nearfar._scaleNear) * ratioDist);
                    verticesNew[idxVer] = Vector3.Scale(vertexdist._pos, ratioScale) + translate;
                }
            }
            else
            {
                for (int idxVer = 0; idxVer < meshSrc.vertices.Length; ++idxVer)
                {
                    verticesNew[idxVer] = (Vector3.Scale(meshSrc.vertices[idxVer], scale)) + translate;
                }
            }
        }

        Mesh meshNew = new Mesh();
        meshNew.vertices = verticesNew;
        meshNew.triangles = trianglesNew;
        meshNew.uv = uvNew;
        meshNew.colors = colorNew;
        meshNew.tangents = tangentsNew;

        meshNew.RecalculateNormals();
        meshNew.RecalculateBounds();

        return meshNew;
    }

    delegate void AttachColliderWhichtype(  GameObject gameObject,
                                            Mesh shapeCollider,
                                            CNearFarParam nearfar,
                                            ref Mesh meshForRender);

    void AttachColliderMeshType(    GameObject gameObject,
                                    Mesh shapeCollider,
                                    CNearFarParam nearfar,
                                    ref Mesh meshForRender)
    {
        MeshCollider meshcollider = gameObject.AddComponent<MeshCollider>();
        meshcollider.sharedMesh = copymeshFromNearToFar(shapeCollider, _sizeCollider, _positionCollider, nearfar);
        meshcollider.transform.position = _positionCollider;
        meshcollider.convex = true;
        meshcollider.isTrigger = true;

        _collider = meshcollider;

        meshForRender = meshcollider.sharedMesh;
    }

    void AttachColliderSphere(  GameObject gameObject,
                                Mesh shapeCollider,
                                CNearFarParam nearfar,
                                ref Mesh meshForRender)
    {
        SphereCollider spherecollider = gameObject.AddComponent<SphereCollider>();
        spherecollider.center = _positionCollider;
        spherecollider.radius = _sizeCollider.x * 0.5f;
        spherecollider.isTrigger = true;
        _collider = spherecollider;

        Vector3 sizeCollider = new Vector3(_sizeCollider.x,
                                        _sizeCollider.x,
                                        _sizeCollider.x);

        meshForRender = GeometryHelper.copymeshFrom(shapeCollider, Quaternion.identity, sizeCollider, _positionCollider);
    }

    void AttachColliderCapsule( GameObject gameObject,
                                Mesh shapeCollider,
                                CNearFarParam nearfar,
                                ref Mesh meshForRender)
    {
        CapsuleCollider capsulecollider = gameObject.AddComponent<CapsuleCollider>();
        capsulecollider.center = _positionCollider;
        capsulecollider.height = _sizeCollider.y;
        capsulecollider.radius = _sizeCollider.x * 0.5f;
        capsulecollider.isTrigger = true;
        _collider = capsulecollider;

        Vector3 sizeCollider = new Vector3(_sizeCollider.x, (_sizeCollider.y / 2.0f), _sizeCollider.x);

        meshForRender = GeometryHelper.copymeshFrom(shapeCollider, Quaternion.identity, sizeCollider, _positionCollider);
    }

    void AttachColliderBox( GameObject gameObject,
                            Mesh shapeCollider,
                            CNearFarParam nearfar,
                            ref Mesh meshForRender)
    {
        if (true == nearfar._doNeartoFar)
        {
            AttachColliderMeshType(gameObject, shapeCollider, nearfar, ref meshForRender);
        }
        else
        {   
            BoxCollider boxcollider = gameObject.AddComponent<BoxCollider>();

            boxcollider.center = _positionCollider;
            boxcollider.size = _sizeCollider;
            _collider = boxcollider;
            _collider.isTrigger = true;

            meshForRender = GeometryHelper.copymeshFrom(shapeCollider, Quaternion.identity, _sizeCollider, _positionCollider);
        }
    }

    void AttachColliderBox2D(GameObject gameObject,
                            Mesh shapeCollider,
                            CNearFarParam nearfar,
                            ref Mesh meshForRender)
    {
        //this._typecollider = etypeactioncollider.collider_box2D;
        if (true == nearfar._doNeartoFar)
        {
            //Debug.Log("attch box2d 1");
            AttachColliderMeshType(gameObject, shapeCollider, nearfar, ref meshForRender);
        }
        else
        {
            //Debug.Log("attch box2d 2");
            BoxCollider boxcollider = gameObject.AddComponent<BoxCollider>();

            boxcollider.center = _positionCollider;
            boxcollider.size = _sizeCollider;
            _collider = boxcollider;
            _collider.isTrigger = true;

            meshForRender = GeometryHelper.copymeshFrom(shapeCollider, Quaternion.identity, _sizeCollider, _positionCollider);
        }
    }
    void AttachRenderColliderChildNode( GameObject objectParent, 
                                        Mesh meshCollider, 
                                        etypeactioncollider typecollider, 
                                        Material materialCollider)
    {
        if (null != materialCollider)
        {
            _rendercolliderfordebug = new crendercolliderfordebug();
            _rendercolliderfordebug.materialCollider = materialCollider;
            _rendercolliderfordebug.meshCollider = meshCollider;
        }
    }

    void AttachActionCollider(  CShapeActionCollider commonColliderShapes, 
                                etypeactioncollider typecollider, 
                                GameObject gameObject)
    {
        Mesh meshForRender = null;
        _colliderAttachers[typecollider](gameObject, 
                                        commonColliderShapes.shapes[typecollider], 
                                        _nearfarParam, 
                                        ref meshForRender);

        AttachRenderColliderChildNode(  gameObject, 
                                        meshForRender, 
                                        typecollider, 
                                        commonColliderShapes._materialCollider);

        gameObject.rigidbody.collisionDetectionMode = CollisionDetectionMode.ContinuousDynamic;

        enableCollider(false);
    }

    void enableCollider(bool enabled)
    {
        if (edetachtiming.none != _detachMovement && false == enabled)
        {
            _attachablehierarchy.endFixPosition(_collider.transform);
        }

        _collider.enabled = _enabledColliderAction = enabled;
        //if (ownerHandler )
        //    ownerHandler._processingActionCollide2D = enabled;
    }

    void resetParamCollidePart()
    {
        _timeStarted = -1.0f;
        enableCollider(false);
    }

    public void startCollidePart(float timeApp)
    {
        _startedColliderActivation = false;
        _processColliderAction = true;
        _timeStarted = timeApp;

        if (edetachtiming.whenstart == _detachMovement)
            _attachablehierarchy.startFixPosition(_collider.transform);
    }

    public void endCollidePart()
    {
        resetParamCollidePart();
        _processColliderAction = false;
        _startedColliderActivation = false;
    }

    public bool processCollidePart(float timeApp, bool shouldRenderDebugMeshes)
    {
        if (false == _processColliderAction || null == _collider) return false;

        bool isoverTimeBound = timeApp > _timeStarted + _timeCollideDelay + _timeCollideDuration;
        bool islessTimeBound = timeApp < _timeStarted + _timeCollideDelay;
        bool isInTimeBoundary = !islessTimeBound && ((false == _startedColliderActivation)?(true):(!isoverTimeBound));

        if (edetachtiming.whenstart == _detachMovement)
            _attachablehierarchy.processFixPosition(_collider.transform);

        if (true == isInTimeBoundary)
        {
            if (false == _enabledColliderAction)
            {
                _startedColliderActivation = true;
                _attachablehierarchy.startFixPosition(_collider.transform);
                
                //  [11/16/2012 whatsin]
                if ( box2DInfo.bUseBox2D )
                    ownerHandler.UpdateColliderEnterSecondaryCheck2D(box2DInfo.position, box2DInfo.collisionBoxPiercing, box2DInfo.LEFT, box2DInfo.TOP, box2DInfo.RIGHT, box2DInfo.BOTTOM);
                 else
                    enableCollider(true);
                    
           }

            if (edetachtiming.whenactive == _detachMovement)
                _attachablehierarchy.processFixPosition(_collider.transform);
        }
        else if (true == _startedColliderActivation && isoverTimeBound) 
        {
            
            endCollidePart(); 
        }


        

        if (null != _rendercolliderfordebug && _enabledColliderAction && isInTimeBoundary && shouldRenderDebugMeshes)
        {
            Graphics.DrawMesh(_rendercolliderfordebug.meshCollider, _collider.transform.localToWorldMatrix, _rendercolliderfordebug.materialCollider, 0);
        }

        return isInTimeBoundary;
    }

    public void InitActionCollideUnit(CShapeActionCollider commonColliderShapes, GameObject gameObject, CActionColliderHandler handler)
    {
        AttachActionCollider(commonColliderShapes, _typecollider, gameObject);
        _attachablehierarchy.initFixPosition(_collider.transform);

        //Debug.Log("init owner hander " + handler);
        ownerHandler = handler;
    }

    public CActionColliderInstance()
    {
        _colliderAttachers[etypeactioncollider.collider_sphere] = AttachColliderSphere;
        _colliderAttachers[etypeactioncollider.collider_capsule] = AttachColliderCapsule;
        _colliderAttachers[etypeactioncollider.collider_box] = AttachColliderBox;
        _colliderAttachers[etypeactioncollider.collider_cylinder] = AttachColliderMeshType;
        _colliderAttachers[etypeactioncollider.collider_box] = AttachColliderBox2D;
    }
}// public class CActionColliderInstance
