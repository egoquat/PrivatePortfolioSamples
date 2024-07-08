//#define EGOQUAT_ADD_CHARACTERCOLLIDER_RELATION_RENDERPOSITION_121127  //@Renderqueue가 적용 안되는 이유로 잠시 작업 보류.

using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;

public class CharacterTeamRelationHandler : MonoBehaviour
{
    protected class PairTeamCollisionException
    {
        public CharacterActionBrain a1 = null;
        public CharacterActionBrain a2 = null;

        public PairTeamCollisionException(CharacterActionBrain a1, CharacterActionBrain a2)
        {
            this.a1 = a1; this.a2 = a2;
        }

        public bool isPair(CharacterActionBrain a1, CharacterActionBrain a2)
        {
            return (this.a1 == a1 && this.a2 == a2) || (this.a2 == a1 && this.a1 == a2);
        }

        public bool isExist(CharacterActionBrain a)
        {
            if (this.a1 == a || this.a2 == a) return true;
            return false;
        }
    }

    [Serializable]
    protected class ColliderSetter
    {
        public bool AllyPC = true;
        public bool AllyNPC = true;
        public bool AllyTower = true;
        public bool EnermyPC = true;
        public bool EnermyNPC = true;
        public bool EnermyTower = true;
        public bool NeutralityNPC = true;
        public bool NeutralityNPCBoss = true;        
        public bool NeutralityTower = true;
        

        [NonSerialized]
        public bool[] relationSets = new bool[(int)ObjectRelative.Max];
        public void Initialize()
        {
            relationSets[(int)ObjectRelative.ALLY_PC] = AllyPC;
            relationSets[(int)ObjectRelative.ALLY_NPCNormal] = AllyNPC;
            relationSets[(int)ObjectRelative.ALLY_Tower] = AllyTower;
            relationSets[(int)ObjectRelative.ENERMY_PC] = EnermyPC;
            relationSets[(int)ObjectRelative.ENERMY_NPCNormal] = EnermyNPC;
            relationSets[(int)ObjectRelative.ENERMY_Tower] = EnermyTower;
            relationSets[(int)ObjectRelative.NEUTRALITY_NPCNormal] = NeutralityNPC;
            relationSets[(int)ObjectRelative.NEUTRALITY_NPCBoss] = NeutralityNPCBoss;
            relationSets[(int)ObjectRelative.NEUTRALITY_Tower] = NeutralityTower;
        }

        public void set(ColliderSetter o)
        {
            AllyPC = o.AllyPC;
            AllyNPC = o.AllyNPC;
            AllyTower = o.AllyTower;
            EnermyPC = o.EnermyPC;
            EnermyNPC = o.EnermyNPC;
            EnermyTower = o.EnermyTower;
            NeutralityNPC = o.NeutralityNPC;
            NeutralityNPCBoss = o.NeutralityNPCBoss;
            NeutralityTower = o.NeutralityTower;
        }
    }

    protected float unitoffset = 0.8f;
    #region team relation rendering order
    public int renderOrderTower = 50;
    public int renderOrderNeutralityBoss = 100;
    public int renderOrderNeutralityNpc = 200;
    public int renderOrderEnermyNpc = 300;
    public int renderOrderAllyNpc = 400;
    public int renderOrderEnermyPc = 500;
    public int renderOrderAllyPc = 600;
    public int renderOrderMine = 700;
    #endregion

    #region team relation collision
    [SerializeField]
    protected ColliderSetter AllyPC = new ColliderSetter();

    [SerializeField]
    protected ColliderSetter AllyNPC = new ColliderSetter();

    [SerializeField]
    protected ColliderSetter Ally_Tower = new ColliderSetter();

    [SerializeField]
    protected ColliderSetter EnermyPC = new ColliderSetter();

    [SerializeField]
    protected ColliderSetter EnermyNPC = new ColliderSetter();

    [SerializeField]
    protected ColliderSetter Enermy_Tower = new ColliderSetter();

    [SerializeField]
    protected ColliderSetter NeutralityNPC = new ColliderSetter();

    [SerializeField]
    protected ColliderSetter NeutralityNPCBoss = new ColliderSetter();

    [SerializeField]
    protected ColliderSetter Neutrality_Tower = new ColliderSetter();

    protected ColliderSetter[] colliderRelations = new ColliderSetter[(int)ObjectRelative.Max];
    private List<CharacterActionBrain> characterInstances = new List<CharacterActionBrain>();
    private List<PairTeamCollisionException> exceptionPairInstances = new List<PairTeamCollisionException>();
    #endregion

    private bool initialized = false;

    //@ 케릭터 삭제 되는 경우 관계 재구성.
    public void RemoveCollisionController(CharacterActionBrain a1)
    {
        if (null == a1) return;

        characterInstances.RemoveAll(a=>a==a1);

        UpdateCollisionRelationAll();
    }

    public bool isExistExceptionRelation(CharacterActionBrain a1, CharacterActionBrain a2)
    {
        return exceptionPairInstances.Exists(p => p.isPair(a1, a2));
    }

    public void UpdateCollisionRelation(CharacterActionBrain a1)
    {
        foreach (PairTeamCollisionException pair in exceptionPairInstances)
        {
            if (true == pair.isExist(a1) && true == pair.a1.characterController.enabled && true == pair.a2.characterController.enabled)
            {
                Physics.IgnoreCollision(pair.a1.characterController.collider, pair.a2.characterController.collider);
            }
        }
    }

    public void UpdateCollisionRelationAll()
    {
        List<CharacterActionBrain> relationDoneList = new List<CharacterActionBrain>();
        foreach(CharacterActionBrain a1 in characterInstances)
        {
            if (false == a1.characterController.enabled) continue;
            Identifier id1 = a1.GetComponent<Identifier>();
            if (null == id1) { Debug.LogError("(null == id1)/" + Time.time); }
            ObjectRelative r1 = id1.GetRelativeWithMine();
            ColliderSetter setter = colliderRelations[(int)r1];
            foreach(CharacterActionBrain a2 in characterInstances)
            {
                if (a1 == a2) continue;
                if (false == a2.characterController.enabled) continue;
                if (true == relationDoneList.Exists(a => a == a2)) continue;

                Identifier id2 = a2.GetComponent<Identifier>();
                if (null == id2) { Debug.LogError("(null == id2)/" + Time.time); }

                ObjectRelative r2 = id2.GetRelativeWithMine();

                bool checkCollision = setter.relationSets[(int)r2];
                if (false == checkCollision)
                {
                    //Debug.Log("--------> Set Physics.Ignore (1:" + (a1.gameObject.name) + "/relative(" 
                    //    + (r1) + "))//2:" + (a2.gameObject.name) + "/relative(" + (r2) + "))/" + Time.time); //testdebug
                    Physics.IgnoreCollision(a1.characterController.collider, a2.characterController.collider);
                }
            }
            relationDoneList.Add(a1);
        }
    }

    //@ Character Collision Relation ship Table
    public void AddCollisionRelation(CharacterActionBrain a1)
    {
        if (false == initialized) { Debug.LogError("false == initialized/" + Time.time); }

        Identifier id1 = a1.GetComponent<Identifier>();
        if (null == id1) { Debug.LogError("(null == id1)/" + Time.time); }

        ObjectRelative relative = id1.GetRelativeWithMine();
        if ((int)relative >= colliderRelations.Length || 0 > (int)relative)
        { Debug.LogError("Range over (relative(" + (relative) + ")== ObjectRelative.NONE)/a1(" + (a1.gameObject.name) + ")/" + Time.time); } 

        ColliderSetter colliderset = colliderRelations[(int)relative];

        if (relative == ObjectRelative.NONE) { Debug.LogError("(relative(" + (relative) + ")== ObjectRelative.NONE)/Unit(" + (a1.gameObject.name) + ")/" + Time.time); }
        if (null == colliderset) 
        {
            Debug.LogError("length(" + (colliderRelations.Length) + ")/null == colliderset/relative(" + (relative) + ")/idx(" + ((int)relative) + ")");
        }

#if EGOQUAT_ADD_CHARACTERCOLLIDER_RELATION_RENDERPOSITION_121127
        #region region_offsetdepth
        LockPosition lockposition = a1.GetComponent<LockPosition>();
        if (null == lockposition) { Debug.LogError("(null == lockposition)/" + Time.time); }

        float positionDepth = 0.0f;
        if (true == GameRule.gr.IsObjectTypeMine()(a1.gameObject))
        {
            positionDepth = (Mathf.Clamp(renderDepthHeroCharacter, 1, 10) * unitoffset);
        }
        else
        {
            positionDepth = (Mathf.Clamp(colliderset.renderDepth, 1, 10) * unitoffset);
        }
        lockposition.setPositionOffsetDepth(positionDepth);

        #endregion
#endif

        #region region_collisionignore_setting
        characterInstances.ForEach(a => {
            if (a != a1)
            {
                ObjectRelative rother = a.GetComponent<Identifier>().GetRelativeWithMine();

                bool checkCollision = colliderset.relationSets[(int)rother];

                //Debug.Log("--> Relative object(" + (a1.name) + ")/ other(" + (rother) + ")/Collision(" + (checkCollision) + ")/" + Time.time);  //testdebug

                if (false == checkCollision && true == a.characterController.enabled && true == a1.characterController.enabled)
                {
                    Physics.IgnoreCollision(a.characterController.collider, a1.characterController.collider);
                    exceptionPairInstances.Add(new PairTeamCollisionException(a1, a));

                    //Debug.Log("----> Physics.Ignore relative(1:" + (a.name) + "/2:" + (a1.name) + ")/ Team1(" + (rother) + ") Team2(" + (a1.GetComponent<Identifier>().GetRelativeWithMine()) + ") /" + Time.time); //testdebug
                }
            }
        });
        if (false == characterInstances.Exists(c => c == a1)) characterInstances.Add(a1);
        #endregion
    }

    //@ Instanced Identifier Units Table

    //@ Function Add new Unit To Units table with Setting Relation Ship

	// Use this for initialization
	void Awake () {
        for (int idx = 0; idx < (int)ObjectRelative.Max; ++idx)
        {
            colliderRelations[idx] = new ColliderSetter();
        }

        colliderRelations[(int)ObjectRelative.ALLY_PC].set(AllyPC);
        colliderRelations[(int)ObjectRelative.ALLY_NPCNormal].set(AllyNPC);
        colliderRelations[(int)ObjectRelative.ENERMY_PC].set(EnermyPC);
        colliderRelations[(int)ObjectRelative.ENERMY_NPCNormal].set(EnermyNPC);
        colliderRelations[(int)ObjectRelative.NEUTRALITY_NPCNormal].set(NeutralityNPC);
        colliderRelations[(int)ObjectRelative.NEUTRALITY_NPCBoss].set(NeutralityNPCBoss);
        colliderRelations[(int)ObjectRelative.ALLY_Tower].set(Ally_Tower);
        colliderRelations[(int)ObjectRelative.ENERMY_Tower].set(Enermy_Tower);
        colliderRelations[(int)ObjectRelative.NEUTRALITY_Tower].set(Neutrality_Tower);

        colliderRelations.Each(c => { if (null!=c) { c.Initialize(); } });

        initialized = true;
	}
	
	// Update is called once per frame
	void Update () {
	
	}
}
