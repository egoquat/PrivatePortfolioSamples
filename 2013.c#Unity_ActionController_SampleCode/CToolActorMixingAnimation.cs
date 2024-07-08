#define EGOQUAT_OPT_UPDATEDELTA_120911

using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class CToolActorMixingAnimation : MonoBehaviour {
    public class CMixingAnimation
    {
        public float _timestartMixAnimation = 0.0f;
        public string _nameMixAnimation = "";
        public AnimationState _anistate = null;
        public float _speedMixAnimation = 1.0f;
        public float _weightTargetBlend = 1.0f;
        public float _timeFadeBlend = 0.1f;

        public void setmixing(float timetostart, AnimationState anistate)
        {
            _timestartMixAnimation = timetostart;
            _anistate = anistate;
            _nameMixAnimation = anistate.name;
        }

        public void clearmixing()
        {
            _anistate = null;
            _nameMixAnimation = "";
        }
    }// public class CMixingAnimation

    protected CharacterActionBrain _actionbrain = null;
    public CMixingAnimation _mixinganimation = new CMixingAnimation();
    bool _requestedMixingAnimation = false;
    float _ratioRequestMixing = 0.0f;
    public void requestMixingAnimation(float ratioReqestMixing)
    {
        if (ratioReqestMixing < 0.0f || ratioReqestMixing > 1.0f) return;
        _requestedMixingAnimation = true;
        _ratioRequestMixing = ratioReqestMixing;
    }

    bool _processMixingAnimation = false;
    public bool isprocessMixingAnimation { get { return _processMixingAnimation; } set { _processMixingAnimation = value; } }
    public bool setprocessMixingAnimation { set { _processMixingAnimation = value; } } 

    public float setRatioTimeProcess
    {
        set
        {
            if (null == _mixinganimation._anistate) return;
            float ratiotimeProcessRequest = Mathf.Clamp01(value);
            _mixinganimation._anistate.normalizedTime = ratiotimeProcessRequest;
        }
    }

    public AnimationState getMixingAnimation { get { return _mixinganimation._anistate; } }

    public bool setnewMixinganimation(  float timestartMixingAni, 
                                        string nameAnimationMixingSrc, 
                                        List<Transform> listTMMixing, 
                                        bool isDescendantMixing)
    {
        if (listTMMixing.Count < 1) return false;
        AnimationState anistateMixingSource = animation[nameAnimationMixingSrc];
        if(null == anistateMixingSource) { Debug.LogError("(null == anistateMixingSource)/"); return false;}

        string nameAniMixing = nameAnimationMixingSrc + "_mixing";

        animation.AddClip(anistateMixingSource.clip, nameAniMixing);
        
        AnimationState anistatemixing = animation[nameAniMixing];
        anistatemixing.layer = 10;
        anistatemixing.wrapMode = WrapMode.Clamp;

        foreach (Transform tmmixingnode in listTMMixing)
        {
            anistatemixing.AddMixingTransform(tmmixingnode, isDescendantMixing);
        }

        _mixinganimation.setmixing(timestartMixingAni, anistatemixing);
        _processMixingAnimation = false;
        return true;
    }

    public void deleteMixinganimation()
    {
        if (null == _mixinganimation._anistate) return;
        animation.RemoveClip(_mixinganimation._anistate.clip);

        _mixinganimation.clearmixing();
    }

    public void stopMixingAnimation()
    {
        if (null == _mixinganimation._anistate) return;
        _mixinganimation._anistate.speed = 0.0f;

        StopAllCoroutines();
    }

    IEnumerator processMixingAnimationCoroutine()
    {
        if (true == _processMixingAnimation)
            _mixinganimation._anistate.speed = _mixinganimation._speedMixAnimation;

        yield return this.WaitForAnimationStateToLastFrame(_mixinganimation._anistate);
        animation.Blend(_mixinganimation._anistate.name, _mixinganimation._weightTargetBlend, _mixinganimation._timeFadeBlend);
    }

    void processMixingAnimation()
    {
        if (null == _mixinganimation._anistate) return;
        if (true == _processMixingAnimation)
        {
            _mixinganimation._anistate.speed = _mixinganimation._speedMixAnimation;
        }
        else
        {
            if (_ratioRequestMixing >= 0.0f || _ratioRequestMixing <= 1.0f)
            {
                _mixinganimation._anistate.normalizedTime = _ratioRequestMixing;
            }
        }

        if (_ratioRequestMixing > 0.5f)
        {
            bool rotationTwiceandMore = _mixinganimation._anistate.normalizedTime < 0.1f;
            if (true == rotationTwiceandMore) return;
        }

        animation.Blend(_mixinganimation._anistate.name, _mixinganimation._weightTargetBlend, _mixinganimation._timeFadeBlend);
    }

    void Awake()
    {
        _actionbrain = GetComponent<CharacterActionBrain>();
    }

	// Use this for initialization
	void Start () {
	    
	}
	
	// Update is called once per frame
	void Update () {
        if (null == _mixinganimation._anistate) return;
        if (false == _processMixingAnimation)
        {
            stopMixingAnimation();
        }

        if (true == _requestedMixingAnimation)
        {
            processMixingAnimation();
            _requestedMixingAnimation = false;
        }
	}
}
